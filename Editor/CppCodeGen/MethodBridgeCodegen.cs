using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using dnlib.DotNet;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{

    public class MethodBindingInfo
    {
        public MethodDef methodDef;

        public string uniqueName;

        public string stubName;

        public List<ParamMarshalInfo> parameters;
        public LuaMarshalMetaInfo returnMetaInfo;

        public string BridgeMethodName => $"Bridge_{uniqueName}";
    }

    public class MethodBridgeAnalyzer
    {
        public MethodBridgeAnalyzer()
        {
        }

        public List<MethodBindingInfo> Collect(AssemblyCache assemblyCache, List<string> assemblyNames)
        {
            var results = new List<MethodBindingInfo>();

            foreach (string assName in assemblyNames)
            {
                ModuleDef mod = assemblyCache.LoadModule(assName);
                foreach (TypeDef typeDef in mod.GetTypes())
                {
                    if (typeDef.HasGenericParameters)
                    {
                        continue;
                    }
                    foreach (MethodDef methodDef in typeDef.Methods)
                    {
                        if (methodDef.HasGenericParameters)
                        {
                            continue;
                        }
                        if (methodDef.IsStaticConstructor)
                        {
                            continue;
                        }
                        var parameters = methodDef.Parameters.Select(p => MarshalMetaUtil.CreateParamMarshalInfo(p, true)).Skip(methodDef.IsStatic ? 0 : 1).ToList();
                        for (int i = 0; i < parameters.Count; i++)
                        {
                            parameters[i].indexExcludedThis = i;
                        }
                        results.Add(new MethodBindingInfo
                        {
                            methodDef = methodDef,
                            uniqueName = NameUtil.CreateUniqueName(methodDef, null, null),
                            stubName = NameUtil.CreateStubName(methodDef, null, null),
                            parameters = parameters,
                            returnMetaInfo = MarshalMetaUtil.CreateMarshalMetaInfo(methodDef.Parameters.ReturnParameter.Type, methodDef.Parameters.ReturnParameter.ParamDef, true),
                        });
                    }
                }
            }
            return results;
        }
    }

    public class MethodBridgeCodegen
    {
        private readonly AssemblyCache _assemblyCache;
        private readonly List<string> _assemblyNames;
        private readonly string _outputDir;

        public MethodBridgeCodegen(AssemblyCache assemblyCache, List<string> assemblyNames, string outputDir)
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
            writer.WriteLine("namespace methodbridge");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"extern const {ConstStrings.nameMethodBridgeEntry} g_methodBridges[];");
            writer.DecreaseIndent();
            writer.WriteLine("} // namespace methodbridge");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "MethodBridgeStub.h"));
        }
        private void GenerateSource()
        {
            var analyzer = new MethodBridgeAnalyzer();
            List<MethodBindingInfo> originalBindings = analyzer.Collect(_assemblyCache, _assemblyNames);
            var sortedBindings = originalBindings.Select(b => (b, $"{b.methodDef.DeclaringType.FullName}.{b.methodDef.Name}"))
            .OrderBy(x => x.Item2).Select(x => x.Item1).ToList();
                        var effectiveBindings = new List<MethodBindingInfo>();
            var uniqueStubNames = new HashSet<string>();
            foreach (var binding in sortedBindings)
            {
                if (!uniqueStubNames.Add(binding.stubName))
                {
                    continue;
                }
                effectiveBindings.Add(binding);
            }


            var writer = new CodeWriter();
            writer.WriteLine("#include \"MethodBridgeStub.h\"");
            writer.WriteLine();
            CodegenCommon.AddCommonIncludes(writer);
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace methodbridge");
            writer.WriteLine("{");
            writer.IncreaseIndent();


            var forwardDeclareStructsGenerator = new ForwardDeclareStructsCodeGen();
            foreach (var binding in effectiveBindings)
            {
                forwardDeclareStructsGenerator.CollectStructDefs(binding.methodDef);
            }
            forwardDeclareStructsGenerator.WriteForwardDeclareStructs(writer);

            foreach (var binding in effectiveBindings)
            {
                GenerateMethodBridgeFunctions(writer, binding);
            }

            GenerateMethodBridgeStub(writer, effectiveBindings);
            writer.DecreaseIndent();
            writer.WriteLine("} // namespace methodbridge");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "MethodBridgeStub.cpp"));
        }

        private void GenerateMethodBridgeFunctions(CodeWriter writer, MethodBindingInfo binding)
        {
            TypeSig retType = binding.returnMetaInfo.typeSig;
            string retTypeName = NameUtil.GetTypeName(retType);
            bool isVoidReturn = MetaUtil.IsVoid(retType);
            writer.WriteLine($"// {binding.methodDef.FullName}");
            writer.WriteLine($"static int {binding.BridgeMethodName}(lua_State* L, void* target, int argStart, {ConstStrings.typeConstMethodMarshalCtxPtr} ctx)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"{ConstStrings.typeMethodInfo} method = ctx->method;");
            foreach (var param in binding.parameters)
            {
                string metaExpr = $"ctx->paramsMeta[{param.indexExcludedThis}]";
                writer.WriteLine($"{NameUtil.GetTypeName(param.marshalMetaInfo.typeSig)} {param.name};");
                writer.WriteLine(CodegenCommon.GeneratePopStatement(binding.methodDef, $"argStart + {param.indexExcludedThis}", metaExpr, param.name, param.marshalMetaInfo.typeSig, param.marshalMetaInfo.marshalAsInfo));
            }
            string retvalName = "_retval";
            if (!isVoidReturn)
            {
                writer.WriteLine($"{retTypeName} {retvalName};");
            }

            writer.WriteLine($"if (target != nullptr)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            {
                string paramTypesWithMethodInfo = string.Join(", ",
                new[] { "void*" }.Concat(
    binding.parameters.Select(p => NameUtil.GetTypeName(p.marshalMetaInfo.typeSig))).Concat(new[] { ConstStrings.typeMethodInfo }));
                writer.WriteLine($"typedef {retTypeName} (*_FnInvokeMethod)({paramTypesWithMethodInfo});");
                writer.WriteLine($"_FnInvokeMethod fnInvokeMethod = (_FnInvokeMethod)method->methodPointer;");

                string paramNamesWithMethodInfo = string.Join(", ", new[] { "target" }.Concat(binding.parameters.Select(p => p.name)).Concat(new[] { "method" }));
                if (!isVoidReturn)
                {
                    writer.WriteLine($"{retvalName} = fnInvokeMethod({paramNamesWithMethodInfo});");
                }
                else
                {
                    writer.WriteLine($"fnInvokeMethod({paramNamesWithMethodInfo});");
                }
            }
            writer.DecreaseIndent();
            writer.WriteLine("}");
            writer.WriteLine("else");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            {
                string paramTypesWithMethodInfo = string.Join(", ",
        binding.parameters.Select(p => NameUtil.GetTypeName(p.marshalMetaInfo.typeSig)).Concat(new[] { ConstStrings.typeMethodInfo }));
                writer.WriteLine($"typedef {retTypeName} (*_FnInvokeMethod)({paramTypesWithMethodInfo});");
                writer.WriteLine($"_FnInvokeMethod fnInvokeMethod = (_FnInvokeMethod)method->methodPointer;");

                string paramNamesWithMethodInfo = string.Join(", ", binding.parameters.Select(p => p.name).Concat(new[] { "method" }));
                if (!isVoidReturn)
                {
                    writer.WriteLine($"{retvalName} = fnInvokeMethod({paramNamesWithMethodInfo});");
                }
                else
                {
                    writer.WriteLine($"fnInvokeMethod({paramNamesWithMethodInfo});");
                }
            }
            writer.DecreaseIndent();
            writer.WriteLine("}");
            if (!isVoidReturn)
            {
                writer.WriteLine(CodegenCommon.GeneratePushStatement(binding.methodDef, "ctx->retMeta", retvalName, retType, binding.returnMetaInfo.marshalAsInfo));
                writer.WriteLine($"return 1;");
            }
            else
            {
                writer.WriteLine($"return 0;");
            }

            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private void GenerateMethodBridgeStub(CodeWriter writer, List<MethodBindingInfo> bindings)
        {
            writer.WriteLine($"const {ConstStrings.nameMethodBridgeEntry} g_methodBridges[] = {{");
            writer.IncreaseIndent();
            foreach (var binding in bindings)
            {
                writer.WriteLine($"{{ \"{binding.stubName}\", {binding.BridgeMethodName}, }},");
            }
            writer.WriteLine("{ nullptr, nullptr },");
            writer.DecreaseIndent();
            writer.WriteLine("};");
        }
    }
}
