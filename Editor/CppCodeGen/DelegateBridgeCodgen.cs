using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.ComTypes;
using dnlib.DotNet;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{
    public class DelegateBindingInfo
    {
        public TypeSig delegateType;

        public string uniqueName;
        public string stubName;

        public MethodDesc invokeMethod;

        public string InvokeImplMethodName => $"InvokeImpl_{uniqueName}";
    }

    public class DelegateBridgeAnalyzer
    {
        public DelegateBridgeAnalyzer()
        {
        }


        private MethodDef FindInvokeMethod(TypeDef typeDef)
        {
            foreach (MethodDef methodDef in typeDef.Methods)
            {
                if (methodDef.Name == "Invoke")
                {
                    return methodDef;
                }
            }
            return null;
        }

        private DelegateBindingInfo Create(TypeSig type)
        {
            MethodDef invokeMethod = FindInvokeMethod(type.ToTypeDefOrRef().ResolveTypeDefThrow());
            GenericInstSig gis = type as GenericInstSig;
            List<TypeSig> klassInst = null;
            if (gis != null)
            {
                klassInst = gis.GenericArguments.ToList();
            }
            return new DelegateBindingInfo
            {
                delegateType = type,
                uniqueName = NameUtil.CreateUniqueName(invokeMethod, klassInst, null),
                stubName = NameUtil.CreateDelegateInvokeSignature(invokeMethod, klassInst, null),
                invokeMethod = MethodDesc.CreateMethodDesc(invokeMethod, klassInst, null, false),
            };
        }

        public List<DelegateBindingInfo> Collect(AssemblyCache assemblyCache, List<string> assemblyNames)
        {
            var results = new List<DelegateBindingInfo>();

            foreach (string assName in assemblyNames)
            {
                ModuleDefMD mod = assemblyCache.LoadModule(assName);
                foreach (TypeDef typeDef in mod.GetTypes())
                {
                    // Include non-public (nested) delegates so GetFunction<T> bridges are generated.
                    if (!MetaUtil.IsSubOfMulticastDelegate(typeDef.ToTypeSig()) || typeDef.HasGenericParameters)
                    {
                        continue;
                    }
                    results.Add(Create(typeDef.ToTypeSig()));
                }
                for (uint rid = 1, n = mod.Metadata.TablesStream.TypeSpecTable.Rows; rid <= n; rid++)
                {
                    TypeSpec ts = mod.ResolveTypeSpec(rid);
                    TypeSig sig = ts.TypeSig;
                    if (!MetaUtil.IsSubOfMulticastDelegate(sig) || sig.ContainsGenericParameter)
                    {
                        continue;
                    }
                    results.Add(Create(sig));
                }

            }
            return results;
        }
    }


    public class DelegateBridgeCodgen
    {
        private readonly AssemblyCache _assemblyCache;
        private readonly List<string> _assemblyNames;
        private readonly string _outputDir;

        public DelegateBridgeCodgen(AssemblyCache assemblyCache, List<string> assemblyNames, string outputDir)
        {
            _assemblyCache = assemblyCache;
            _assemblyNames = assemblyNames;
            _outputDir = outputDir;
        }

        public void Generate()
        {
            GenerateHeader();
            GenerateSource();
        }

        private void GenerateHeader()
        {
            var writer = new CodeWriter();
            writer.WriteLine("#pragma once");
            writer.WriteLine();
            writer.WriteLine("#include \"../bridge/BridgeDefs.h\"");
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace delegatebridge");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"extern const {ConstStrings.nameDelegateBridgeEntry} g_delegateBridges[];");
            writer.DecreaseIndent();
            writer.WriteLine("} // namespace delegatebridge");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "DelegateBridgeStub.h"));
        }

        private void GenerateSource()
        {
            var analyzer = new DelegateBridgeAnalyzer();
            List<DelegateBindingInfo> originalBindings = analyzer.Collect(_assemblyCache, _assemblyNames);
            var sortedBindings = originalBindings.Select(b => (b, $"{b.delegateType.FullName}"))
            .OrderBy(x => x.Item2).Select(x => x.Item1).ToList();

            var uniqueStubNames = new HashSet<string>();
            var effectiveBindings = new List<DelegateBindingInfo>();
            foreach (var binding in sortedBindings)
            {
                if (!uniqueStubNames.Add(binding.stubName))
                {
                    continue;
                }
                effectiveBindings.Add(binding);
            }


            var writer = new CodeWriter();
            writer.WriteLine("#include \"DelegateBridgeStub.h\"");
            writer.WriteLine();
            CodegenCommon.AddCommonIncludes(writer);
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace delegatebridge");
            writer.WriteLine("{");
            writer.IncreaseIndent();

            var forwardDeclareStructsGenerator = new ForwardDeclareStructsCodeGen();
            foreach (var binding in effectiveBindings)
            {
                forwardDeclareStructsGenerator.CollectStructDefs(binding.invokeMethod);
            }
            forwardDeclareStructsGenerator.WriteForwardDeclareStructs(writer);

            foreach (var binding in effectiveBindings)
            {
                GenerateDelegateBridgeFunction(writer, binding);
            }
            GenerateDelegateBridgeStub(writer, effectiveBindings);

            writer.DecreaseIndent();
            writer.WriteLine("} // namespace delegatebridge");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "DelegateBridgeStub.cpp"));
        }

        private void GenerateDelegateBridgeFunction(CodeWriter writer, DelegateBindingInfo binding)
        {
            TypeSig returnType = binding.invokeMethod.ReturnInfo.type;
            string returnTypeName = NameUtil.GetTypeName(returnType);
            bool isVoidReturn = MetaUtil.IsVoid(returnType);
            string parameterNameAndTypesWithMethodInfo = string.Join(", ", new[] { "Il2CppObject* target" }
            .Concat(binding.invokeMethod.ParamInfos.Select(p => $"{NameUtil.GetTypeName(p.type)} {p.name}"))
            .Concat(new[] { $"{ConstStrings.typeMethodInfo} method" }));
            writer.WriteLine($"static {returnTypeName} {binding.InvokeImplMethodName}({parameterNameAndTypesWithMethodInfo})");
            writer.WriteLine("{");
            writer.IncreaseIndent();

            writer.WriteLine($"lua_State* L = LuaEnv::GetState();");
            writer.WriteLine("OpaqueParameterScope opaqueScope;");
            writer.WriteLine("LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);");
            writer.WriteLine("const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;");
            writer.WriteLine($"LuaStackGuard guard(L);");
            writer.WriteLine($"int errfunc = LuaEnv::PushErrorHandler();");
            writer.WriteLine($"LuaUtil::PushRef(L, luaMethod->funcRef);");
            writer.WriteLine("(void)method;");

            foreach (var param in binding.invokeMethod.ParamInfos)
            {
                writer.WriteLine(
                    $"ctx->paramsMeta[{param.indexExcludedThis}]->cs2luaWriter(L, &{param.name}, ctx->paramsMeta[{param.indexExcludedThis}]);");
            }

            writer.WriteLine($"LuaUtil::PCall(L, {binding.invokeMethod.ParamInfos.Count}, {(isVoidReturn ? 0 : 1)}, errfunc);");

            if (!isVoidReturn)
            {
                string retvalName = "_retval";
                writer.WriteLine($"{returnTypeName} {retvalName}{{}};");
                writer.WriteLine($"ctx->retMeta->lua2csWriter(L, -1, &{retvalName}, ctx->retMeta);");
                writer.WriteLine($"return {retvalName};");
            }

            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private void GenerateDelegateBridgeStub(CodeWriter writer, List<DelegateBindingInfo> bindings)
        {
            writer.WriteLine($"const {ConstStrings.nameDelegateBridgeEntry} g_delegateBridges[] = {{");
            writer.IncreaseIndent();
            foreach (var binding in bindings)
            {
                writer.WriteLine($"{{ \"{binding.stubName}\", ({ConstStrings.typeIl2CppMethodPointer}){binding.InvokeImplMethodName} }},");
            }
            writer.WriteLine($"{{ nullptr, nullptr }},");
            writer.DecreaseIndent();
            writer.WriteLine($"}};");
        }
    }
}