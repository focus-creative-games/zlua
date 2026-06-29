using System;
using System.Linq;
using dnlib.DotNet;

namespace ZLua
{
    internal static class LuaInvokeMonoReferenceImporter
    {
        internal const string MonoAssemblyName = "ZLua.Mono";
        internal const string CommonAssemblyName = "ZLua.Common";
        internal const string BridgeTypeFullName = "ZLua.LuaInvokeBridge";
        internal const string SiteRegistryTypeFullName = "ZLua.LuaInvokeSiteRegistry";
        internal const string AppDomainTypeFullName = "ZLua.LuaMonoAppDomain";
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

        internal static IMethod ImportRunLuaFuncVoidWithMethodInfo(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            return ImportRunLuaFunc(module, resolver, isGeneric: false);
        }

        internal static IMethod ImportRunLuaFuncGenericWithMethodInfoDefinition(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            return ImportRunLuaFunc(module, resolver, isGeneric: true);
        }

        internal static IMethod ImportGetMethodFromHandle(ModuleDef module, bool withDeclaringType)
        {
            TypeDef objectType = module.CorLibTypes.Object.TypeDefOrRef.ResolveTypeDef();
            TypeDef methodBaseType = objectType?.Module.Find("System.Reflection.MethodBase", isReflectionName: true);
            if (methodBaseType == null)
            {
                return null;
            }

            string methodName = "GetMethodFromHandle";
            foreach (MethodDef candidate in methodBaseType.Methods)
            {
                if (!candidate.IsStatic
                    || !string.Equals(candidate.Name, methodName, StringComparison.Ordinal)
                    || candidate.MethodSig == null)
                {
                    continue;
                }

                int paramCount = candidate.MethodSig.Params.Count;
                if (withDeclaringType && paramCount == 2)
                {
                    return module.Import(candidate);
                }

                if (!withDeclaringType && paramCount == 1)
                {
                    return module.Import(candidate);
                }
            }

            return null;
        }

        internal static ITypeDefOrRef ImportProcessedAttributeType(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            TypeDef attributeType = FindCommonType(resolver, ProcessedAttributeTypeFullName);
            return attributeType != null ? module.Import(attributeType) : null;
        }

        private static IMethod ImportRunLuaFunc(
            ModuleDef module,
            LuaInvokeILPostProcessorAssemblyResolver resolver,
            bool isGeneric)
        {
            TypeDef appDomainType = FindMonoType(resolver, AppDomainTypeFullName);
            if (appDomainType == null)
            {
                return null;
            }

            foreach (MethodDef candidate in appDomainType.Methods)
            {
                if (!candidate.IsStatic
                    || !string.Equals(candidate.Name, "RunLuaFunc", StringComparison.Ordinal)
                    || candidate.MethodSig == null
                    || candidate.HasGenericParameters != isGeneric)
                {
                    continue;
                }

                MethodSig sig = candidate.MethodSig;
                if (sig.Params.Count < 3 || !IsMethodInfoType(sig.Params[0]))
                {
                    continue;
                }

                if (sig.Params[1].ElementType != ElementType.String
                    || sig.Params[2].ElementType != ElementType.String)
                {
                    continue;
                }

                if (!isGeneric && sig.RetType.ElementType != ElementType.Void)
                {
                    continue;
                }

                if (isGeneric && sig.RetType.ElementType == ElementType.Void)
                {
                    continue;
                }

                return module.Import(candidate);
            }

            return null;
        }

        private static bool IsMethodInfoType(TypeSig typeSig)
        {
            ITypeDefOrRef typeRef = typeSig?.ToTypeDefOrRef();
            return typeRef != null
                && string.Equals(typeRef.Name, "MethodInfo", StringComparison.Ordinal)
                && string.Equals(typeRef.Namespace, "System.Reflection", StringComparison.Ordinal);
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
