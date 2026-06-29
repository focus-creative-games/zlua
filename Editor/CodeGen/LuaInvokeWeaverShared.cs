using System;
using dnlib.DotNet;

namespace ZLua
{
    internal static class LuaInvokeWeaverShared
    {
        internal const int CurrentWeaveVersion = 4;

        internal static bool RemoveLuaInvokeAttribute(MethodDef method, CustomAttribute luaInvokeAttr)
        {
            if (luaInvokeAttr == null)
            {
                return false;
            }

            return method.CustomAttributes.Remove(luaInvokeAttr);
        }

        internal static CustomAttribute FindLuaInvokeAttribute(MethodDef method)
        {
            for (int i = 0; i < method.CustomAttributes.Count; i++)
            {
                CustomAttribute attr = method.CustomAttributes[i];
                if (IsAttribute(attr, "ZLua", "LuaInvokeAttribute"))
                {
                    return attr;
                }
            }

            return null;
        }

        internal static bool IsAttribute(CustomAttribute attr, string @namespace, string name)
        {
            ITypeDefOrRef typeRef = attr.AttributeType;
            return typeRef != null &&
                   string.Equals(typeRef.Namespace, @namespace, StringComparison.Ordinal) &&
                   string.Equals(typeRef.Name, name, StringComparison.Ordinal);
        }

        internal static void EnsureAssemblyProcessedAttribute(
            ModuleDefMD module,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            CustomAttribute existing = FindAssemblyProcessedAttribute(module);
            if (existing != null)
            {
                module.Assembly.CustomAttributes.Remove(existing);
            }

            ITypeDefOrRef attributeType = LuaInvokeMonoReferenceImporter.ImportProcessedAttributeType(module, resolver);
            if (attributeType == null)
            {
                return;
            }

            var ctorSig = MethodSig.CreateInstance(module.CorLibTypes.Void, module.CorLibTypes.Int32);
            var ctorRef = new MemberRefUser(module, ".ctor", ctorSig, attributeType);
            var attribute = new CustomAttribute(ctorRef);
            attribute.ConstructorArguments.Add(new CAArgument(module.CorLibTypes.Int32, CurrentWeaveVersion));
            module.Assembly.CustomAttributes.Add(attribute);
        }

        private static CustomAttribute FindAssemblyProcessedAttribute(ModuleDef module)
        {
            for (int i = 0; i < module.Assembly.CustomAttributes.Count; i++)
            {
                CustomAttribute attr = module.Assembly.CustomAttributes[i];
                if (IsAttribute(attr, "ZLua", "LuaInvokeWeaverProcessedAttribute"))
                {
                    return attr;
                }
            }

            return null;
        }
    }
}
