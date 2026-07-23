using System;
using System.Linq;
using dnlib.DotNet;

namespace ZLua
{
    internal static class LuaInvokeMonoReferenceImporter
    {
        internal const string MonoAssemblyName = "ZLua.Mono";
        internal const string CommonAssemblyName = "ZLua.Common";
        internal const string BridgeTypeFullName = "ZLua.Bridge.LuaInvokeBridge";
        internal const string SiteRegistryTypeFullName = "ZLua.Bridge.LuaInvokeSiteRegistry";
        internal const string ProcessedAttributeTypeFullName = "ZLua.LuaInvokeWeaverProcessedAttribute";

        internal static IMethod ImportBridgeMethod(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver,
            string bridgeName,
            TypeSig[] genericArgs)
        {
            if (string.IsNullOrEmpty(bridgeName))
            {
                return null;
            }

            TypeDef bridgeType = FindMonoType(resolver, BridgeTypeFullName);
            if (bridgeType == null)
            {
                return null;
            }

            MethodDef methodDef = bridgeType.Methods.FirstOrDefault(m =>
                m.IsStatic && string.Equals(m.Name, bridgeName, StringComparison.Ordinal));
            if (methodDef == null)
            {
                return null;
            }

            IMethod imported = module.Import(methodDef);
            if (!methodDef.HasGenericParameters)
            {
                return imported;
            }

            if (genericArgs == null || genericArgs.Length != methodDef.GenericParameters.Count)
            {
                return null;
            }

            var genericSig = new GenericInstMethodSig();
            for (int i = 0; i < genericArgs.Length; i++)
            {
                genericSig.GenericArguments.Add(genericArgs[i]);
            }

            return new MethodSpecUser((IMethodDefOrRef)imported, genericSig);
        }

        internal static IMethod ImportGetOrCreateFunctionRef(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            TypeDef siteRegistryType = FindMonoType(resolver, SiteRegistryTypeFullName);
            if (siteRegistryType == null)
            {
                return null;
            }

            MethodDef methodDef = siteRegistryType.Methods.FirstOrDefault(m =>
                m.IsStatic && string.Equals(m.Name, "GetOrCreateFunctionRef", StringComparison.Ordinal));
            return methodDef != null ? module.Import(methodDef) : null;
        }

        internal static ITypeDefOrRef ImportProcessedAttributeType(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            TypeDef attributeType = FindCommonType(resolver, ProcessedAttributeTypeFullName);
            return attributeType != null ? module.Import(attributeType) : null;
        }

        private static TypeDef FindMonoType(LuaInvokeILPostProcessorAssemblyResolver resolver, string fullName)
        {
            AssemblyDef assembly = resolver.ResolveAssembly(MonoAssemblyName);
            return assembly != null ? FindTypeDef(assembly, fullName) : null;
        }

        private static TypeDef FindCommonType(LuaInvokeILPostProcessorAssemblyResolver resolver, string fullName)
        {
            AssemblyDef assembly = resolver.ResolveAssembly(CommonAssemblyName);
            return assembly != null ? FindTypeDef(assembly, fullName) : null;
        }

        private static TypeDef FindTypeDef(AssemblyDef assembly, string fullName)
        {
            if (assembly?.Modules == null)
            {
                return null;
            }

            for (int moduleIndex = 0; moduleIndex < assembly.Modules.Count; moduleIndex++)
            {
                ModuleDef module = assembly.Modules[moduleIndex];
                if (module?.Types == null)
                {
                    continue;
                }

                foreach (TypeDef type in module.GetTypes())
                {
                    if (string.Equals(type.FullName, fullName, StringComparison.Ordinal))
                    {
                        return type;
                    }
                }
            }

            return null;
        }
    }
}
