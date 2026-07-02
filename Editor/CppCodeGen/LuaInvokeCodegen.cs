// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Security.Policy;
using System.Text;
using dnlib.DotNet;
using UnityEngine;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{

    public class LuaInvokeCodegen
    {
        private readonly AssemblyCache _assemblyCache;
        private readonly List<string> _assemblyNames;
        private readonly string _outputDir;

        public LuaInvokeCodegen(AssemblyCache assemblyCache, List<string> assemblyNames, string outputDir)
        {
            _assemblyCache = assemblyCache;
            _assemblyNames = assemblyNames;
            _outputDir = outputDir;
        }

        public void Generate()
        {
            var analyzer = new LuaInvokeAnalyzer();
            List<LuaInvokeBindingInfo> bindings = analyzer.Collect(_assemblyCache, _assemblyNames);

            WriteLuaInvokeSites(bindings);

            string summary =
                $"[LuaInvokeCodegen] Generated {bindings.Count} LuaInvoke binding(s) from {_assemblyNames.Count} user assembly(ies) to: {_outputDir}";
            Debug.Log(summary);
        }

        private void WriteLuaInvokeSites(List<LuaInvokeBindingInfo> bindings)
        {
            GenerateHeader(Path.Combine(_outputDir, "LuaInvokeStub.h"));
            GenerateSource(Path.Combine(_outputDir, "LuaInvokeStub.cpp"), bindings);
        }

        private void GenerateHeader(string outputFile)
        {
            var header = new CodeWriter();
            header.WriteLine("#pragma once");
            header.WriteLine();
            header.WriteLine("#include \"../bridge/LuaInvokeHelper.h\"");
            header.WriteLine();
            header.WriteLine("namespace zlua");
            header.WriteLine("{");
            header.WriteLine("namespace luainvoke");
            header.WriteLine("{");
            header.IncreaseIndent();
            header.WriteLine("void RegisterGeneratedInternalCalls();");
            header.DecreaseIndent();
            header.WriteLine("} // namespace luainvoke");
            header.WriteLine("} // namespace zlua");
            header.Save(outputFile);
        }

        private void GenerateSource(string outputFile, List<LuaInvokeBindingInfo> bindings)
        {
            var writer = new CodeWriter();
            CodegenCommon.AddCommonIncludes(writer);
            writer.WriteLine();
            writer.WriteLine("#include \"vm/Class.h\"");
            writer.WriteLine("#include \"vm/InternalCalls.h\"");
            writer.WriteLine();

            var forwardDeclareStructsGenerator = new ForwardDeclareStructsCodeGen();
            foreach (var binding in bindings)
            {
                forwardDeclareStructsGenerator.CollectStructDefs(binding.methodDef);
            }
            forwardDeclareStructsGenerator.WriteForwardDeclareStructs(writer);

            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace luainvoke");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine();

            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                GenerateInternalCallFunction(binding, writer);
            }

            writer.WriteLine("void RegisterGeneratedInternalCalls()");
            writer.WriteLine("{");
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                writer.WriteLine(
                    $"    il2cpp::vm::InternalCalls::Add(\"{binding.InternalCallName}\", (Il2CppMethodPointer){binding.IcFunctionName});");
            }
            writer.WriteLine("}");
            
            writer.DecreaseIndent();
            writer.WriteLine("} // namespace luainvoke");
            writer.WriteLine("} // namespace zlua");
            writer.Save(outputFile);
        }

        private string GetTypeName(LuaMarshalMetaInfo meta)
        {
            return NameUtil.GetTypeName(meta.typeSig);
        }

        private void GenerateInternalCallFunction(LuaInvokeBindingInfo binding, CodeWriter writer)
        {
            LuaMarshalMetaInfo returnMetaInfo = binding.returnMetaInfo;
            LuaMarshalAsInfo marshalAsInfo = returnMetaInfo.marshalAsInfo;
            TypeSig retType = returnMetaInfo.typeSig;
            string retTypeName = NameUtil.GetTypeName(retType);
            bool isVoidReturn = MetaUtil.IsVoid(retType);
            bool needsOpaqueScope =
                binding.parameters.Any(p => p.marshalMetaInfo.marshalAsInfo != null && p.marshalMetaInfo.marshalAsInfo.marshalType ==
                                            LuaMarshalType.OpaqueLightUserData) ||
                (!isVoidReturn && marshalAsInfo != null && marshalAsInfo.marshalType == LuaMarshalType.OpaqueLightUserData);
            writer.WriteLine($"// {binding.methodDef.FullName}");

            writer.WriteLine($"static bool {binding.InitializedFlagId};");
            writer.WriteLine($"static LuaInvokeSite {binding.SiteId};");
            writer.WriteLine($"static const MethodMarshalCtx* {binding.MethodMarshalCtxId};");
            writer.WriteLine();
            writer.WriteLine(
                $"static {retTypeName} {binding.IcFunctionName}({string.Join(", ", binding.parameters.Select(p => $"{GetTypeName(p.marshalMetaInfo)} {p.name}"))})");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine($"if (!{binding.InitializedFlagId})");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine(
                $"LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), \"{binding.luaModuleName}\", \"{binding.luaFunctionName}\", \"{binding.methodDef.Module.Assembly.Name}\", \"{binding.InternalCallName}\",");
            writer.WriteLine($"        {binding.SiteId}, {binding.MethodMarshalCtxId});");
            writer.WriteLine($"{binding.InitializedFlagId} = true;");
            writer.DecreaseIndent();
            writer.WriteLine("}");

            string retvalName = isVoidReturn ? string.Empty : "_retval";
            if (!isVoidReturn)
            {
                writer.WriteLine($"{retTypeName} {retvalName}{{}};");
            }

            writer.WriteLine("lua_State* L = LuaEnv::GetState();");
            writer.WriteLine("LuaStackGuard guard(L);");
            if (needsOpaqueScope)
            {
                writer.WriteLine("    OpaqueParameterScope opaqueScope;");
            }
            writer.WriteLine($"int errfunc = LuaEnv::PushErrorHandler();");
            writer.WriteLine($"LuaUtil::PushRef(L, {binding.SiteId}.funcRef);");

            foreach (var param in binding.parameters)
            {
                string metaExpr = $"{binding.MethodMarshalCtxId}->paramsMeta[{param.indexExcludedThis}]";
                writer.WriteLine($"{CodegenCommon.GeneratePushStatement(binding.methodDef, metaExpr, param.name, param.marshalMetaInfo.typeSig, param.marshalMetaInfo.marshalAsInfo)}");
            }

            int resultCount = isVoidReturn ? 0 : 1;
            writer.WriteLine($"LuaUtil::PCall(L, {binding.parameters.Count}, {resultCount}, errfunc);");

            if (!isVoidReturn)
            {
                string metaExpr = $"{binding.MethodMarshalCtxId}->retMeta";
                writer.WriteLine($"{CodegenCommon.GeneratePopStatement(binding.methodDef, "-1", metaExpr, retvalName, retType, marshalAsInfo)}");
                writer.WriteLine($"return {retvalName};");
            }

            writer.DecreaseIndent();
            writer.WriteLine("}");
        }


    }
}
