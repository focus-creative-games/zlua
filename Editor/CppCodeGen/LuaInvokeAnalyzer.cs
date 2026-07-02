// Copyright 2026 Code Philosophy

using dnlib.DotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{

    public class LuaInvokeAnalyzer
    {

        public LuaInvokeAnalyzer()
        {
        }

        public List<LuaInvokeBindingInfo> Collect(AssemblyCache assemblyCache, List<string> assemblyNames)
        {
            var results = new List<LuaInvokeBindingInfo>();

            foreach (string assName in assemblyNames)
            {
                ModuleDef mod = assemblyCache.LoadModule(assName);
                if (mod.IsCoreLibraryModule == true || assName.StartsWith("System."))
                {
                    continue;
                }
                CollectFromAssembly(mod, results);
            }

            results.Sort((a, b) => string.CompareOrdinal(a.SiteId, b.SiteId));
            return results;
        }


        private bool TryGetLuaInvokeAttribute(MethodDef method, out CustomAttribute ca)
        {
            ca = null;
            if (!method.HasCustomAttributes)
            {
                return false;
            }
            foreach (var c in method.CustomAttributes)
            {
                if (c.TypeFullName == typeof(LuaInvokeAttribute).FullName)
                {
                    ca = c;
                    return true;
                }
            }
            return false;
        }

        private void CollectFromAssembly(
            ModuleDef assembly,
            List<LuaInvokeBindingInfo> results)
        {
            foreach (TypeDef typeDef in assembly.GetTypes())
            {
                foreach (MethodDef methodDef in typeDef.Methods)
                {
                    if (!TryGetLuaInvokeAttribute(methodDef, out var ca))
                    {
                        continue;
                    }
                    LuaInvokeBindingInfo binding = CreateBinding(methodDef, ca);
                    results.Add(binding);
                }
            }
        }

        private bool TryGetModuleAndMethodName(CustomAttribute ca, out string module, out string function)
        {
            module = null;
            function = null;
            if (ca == null || ca.ConstructorArguments.Count < 2)
            {
                return false;
            }
            module = ca.ConstructorArguments[0].Value as UTF8String;
            function = ca.ConstructorArguments[1].Value as UTF8String;
            return !string.IsNullOrWhiteSpace(module) && !string.IsNullOrWhiteSpace(function);
        }

        private LuaInvokeBindingInfo CreateBinding(MethodDef method, CustomAttribute ca)
        {
            if (!method.IsStatic)
            {
                throw new CodeGenException($"Method {method.FullName} is not static.");
            }
            if (!method.IsInternalCall)
            {
                throw new CodeGenException($"Method {method.FullName} should be an internal call.");
            }

            if (!TryGetModuleAndMethodName(ca, out string moduleName, out string functionName))
            {
                throw new CodeGenException($"Method {method.FullName} has an invalid LuaInvokeAttribute.");
            }

            var binding = new LuaInvokeBindingInfo();
            binding.methodDef = method;
            binding.luaModuleName = moduleName;
            binding.luaFunctionName = functionName;
            binding.uniqueName = NameUtil.CreateUniqueName(method, null, null);
            binding.internalCallSignature = NameUtil.CreateInternalCallSignature(method);
            binding.returnMetaInfo = MarshalMetaUtil.CreateMarshalMetaInfo(method.MethodSig.RetType, method.Parameters.ReturnParameter.ParamDef, false);
            binding.parameters = method.Parameters.Select(p => MarshalMetaUtil.CreateParamMarshalInfo(p, false)).ToList();
            return binding;
        }
    }
}
