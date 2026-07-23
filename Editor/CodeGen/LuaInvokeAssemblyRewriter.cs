using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using dnlib.DotNet;
using dnlib.DotNet.Emit;
using MethodAttributes = dnlib.DotNet.MethodAttributes;
using MethodImplAttributes = dnlib.DotNet.MethodImplAttributes;

namespace ZLua
{
    internal static class LuaInvokeAssemblyRewriter
    {
        private const int MethodImplOptionsInternalCall = (int)MethodImplOptions.InternalCall;

        internal static bool Process(
            ModuleDefMD module,
            bool isEditorDevelopment,
            LuaInvokeILPostProcessorAssemblyResolver resolver,
            out string errorMessage)
        {
            errorMessage = null;
            bool changed = false;

            try
            {
                IMethod getOrCreateFunctionRefMethod = isEditorDevelopment
                    ? LuaInvokeMonoReferenceImporter.ImportGetOrCreateFunctionRef(module, resolver)
                    : null;

                foreach (TypeDef type in module.GetTypes())
                {
                    foreach (MethodDef method in type.Methods)
                    {
                        CustomAttribute luaInvokeAttr = LuaInvokeWeaverShared.FindLuaInvokeAttribute(method);
                        if (luaInvokeAttr == null)
                        {
                            continue;
                        }

                        (string moduleName, string methodName) = ReadLuaInvokeNames(luaInvokeAttr, method);
                        ValidateLuaInvokeMethod(method);

                        if (isEditorDevelopment)
                        {
                            if (TryRewriteEditorMethod(
                                    module,
                                    method,
                                    luaInvokeAttr,
                                    moduleName,
                                    methodName,
                                    getOrCreateFunctionRefMethod,
                                    resolver))
                            {
                                changed = true;
                                continue;
                            }

                            throw new InvalidOperationException(
                                $"[LuaInvoke] no typed bridge for {method.FullName}. "
                                + "Add a matching ZLua.Bridge.LuaInvokeBridge catalog entry or adjust LuaMarshalAs; "
                                + "default struct types (e.g. UnityEngine.Vector3) are not supported.");
                        }
                        else
                        {
                            RewritePlayerMethod(module, method, luaInvokeAttr);
                        }

                        changed = true;
                    }
                }
            }
            catch (Exception ex)
            {
                errorMessage = ex.ToString();
                return false;
            }

            if (changed)
            {
                LuaInvokeWeaverShared.EnsureAssemblyProcessedAttribute(module, resolver);
            }

            return changed;
        }

        private static void ValidateLuaInvokeMethod(MethodDef method)
        {
            if (!method.IsStatic)
            {
                throw new InvalidOperationException($"[LuaInvoke] method must be static: {method.FullName}");
            }

            if (method.HasBody)
            {
                throw new InvalidOperationException($"[LuaInvoke] method must be extern: {method.FullName}");
            }

            if (method.DeclaringType != null && method.DeclaringType.HasGenericParameters)
            {
                throw new InvalidOperationException($"[LuaInvoke] cannot be declared on generic type: {method.FullName}");
            }

            if (method.HasGenericParameters)
            {
                throw new InvalidOperationException($"[LuaInvoke] method cannot be generic: {method.FullName}");
            }
        }

        private static (string moduleName, string methodName) ReadLuaInvokeNames(CustomAttribute luaInvokeAttr, MethodDef method)
        {
            if (luaInvokeAttr.ConstructorArguments.Count < 2)
            {
                throw new InvalidOperationException($"[LuaInvoke] requires moduleName and methodName: {method.FullName}");
            }

            string moduleName = luaInvokeAttr.ConstructorArguments[0].Value as UTF8String;
            string methodName = luaInvokeAttr.ConstructorArguments[1].Value as UTF8String;

            if (string.IsNullOrWhiteSpace(moduleName) || string.IsNullOrWhiteSpace(methodName))
            {
                throw new InvalidOperationException($"[LuaInvoke] requires non-empty moduleName and methodName: {method.FullName}");
            }

            return (moduleName, methodName);
        }

        private static bool TryRewriteEditorMethod(
            ModuleDefMD module,
            MethodDef method,
            CustomAttribute luaInvokeAttr,
            string moduleName,
            string methodName,
            IMethod getOrCreateFunctionRefMethod,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            if (getOrCreateFunctionRefMethod == null)
            {
                return false;
            }

            if (!LuaInvokeBridgeCatalog.TryResolveBridgeMethod(module, method, resolver, out IMethod bridgeMethod))
            {
                return false;
            }

            return LuaInvokeWeaverFastPath.TryRewriteEditorMethod(
                module,
                method,
                luaInvokeAttr,
                moduleName,
                methodName,
                bridgeMethod,
                getOrCreateFunctionRefMethod);
        }

        private static void RewritePlayerMethod(ModuleDefMD module, MethodDef method, CustomAttribute luaInvokeAttr)
        {
            RemoveDllImportAttributeIfExists(method);

            method.Body = null;
            method.ImplMap = null;
            method.Attributes &= ~MethodAttributes.PinvokeImpl;
            method.ImplAttributes &= ~MethodImplAttributes.Managed;
            method.ImplAttributes &= ~MethodImplAttributes.IL;
            method.ImplAttributes |= MethodImplAttributes.InternalCall;

            AddMethodImplInternalCallAttribute(module, method);
        }

        private static void AddMethodImplInternalCallAttribute(ModuleDefMD module, MethodDef method)
        {
            TypeRef methodImplTypeRef = module.CorLibTypes.GetTypeRef("System.Runtime.CompilerServices", "MethodImplAttribute");
            var ctorSig = MethodSig.CreateInstance(module.CorLibTypes.Void, module.CorLibTypes.Int32);
            var ctorRef = new MemberRefUser(module, ".ctor", ctorSig, methodImplTypeRef);

            var attribute = new CustomAttribute(ctorRef);
            attribute.ConstructorArguments.Add(new CAArgument(module.CorLibTypes.Int32, MethodImplOptionsInternalCall));
            method.CustomAttributes.Add(attribute);
        }

        private static void RemoveDllImportAttributeIfExists(MethodDef method)
        {
            for (int i = method.CustomAttributes.Count - 1; i >= 0; i--)
            {
                if (LuaInvokeWeaverShared.IsAttribute(method.CustomAttributes[i], "System.Runtime.InteropServices", "DllImportAttribute"))
                {
                    method.CustomAttributes.RemoveAt(i);
                }
            }
        }
    }
}
