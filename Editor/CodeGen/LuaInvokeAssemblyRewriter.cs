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

                            if (TryRewriteLegacyEditorMethod(module, method, luaInvokeAttr, moduleName, methodName, resolver))
                            {
                                changed = true;
                                continue;
                            }

                            throw new InvalidOperationException(
                                $"[LuaInvoke] failed to weave {method.FullName}. Ensure ZLua.Mono is available to the compiler.");
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

        private static bool TryRewriteLegacyEditorMethod(
            ModuleDefMD module,
            MethodDef method,
            CustomAttribute luaInvokeAttr,
            string moduleName,
            string methodName,
            LuaInvokeILPostProcessorAssemblyResolver resolver)
        {
            IMethod getMethodFromHandleWithType = LuaInvokeMonoReferenceImporter.ImportGetMethodFromHandle(module, withDeclaringType: true);
            IMethod getMethodFromHandle = LuaInvokeMonoReferenceImporter.ImportGetMethodFromHandle(module, withDeclaringType: false);
            IMethod runVoidMethod = LuaInvokeMonoReferenceImporter.ImportRunLuaFuncVoidWithMethodInfo(module, resolver);
            IMethod runGenericDef = LuaInvokeMonoReferenceImporter.ImportRunLuaFuncGenericWithMethodInfoDefinition(module, resolver);

            if (runVoidMethod == null || runGenericDef == null)
            {
                return false;
            }

            LuaInvokeWeaverShared.RemoveLuaInvokeAttribute(method, luaInvokeAttr);

            method.ImplMap = null;
            method.Attributes &= ~MethodAttributes.PinvokeImpl;
            method.ImplAttributes &= ~MethodImplAttributes.InternalCall;
            method.ImplAttributes &= ~MethodImplAttributes.Native;
            method.ImplAttributes |= MethodImplAttributes.Managed | MethodImplAttributes.IL;

            var body = new CilBody { InitLocals = false };
            body.Instructions.Add(Instruction.Create(OpCodes.Ldtoken, method));
            if (getMethodFromHandleWithType != null && method.DeclaringType != null)
            {
                body.Instructions.Add(Instruction.Create(OpCodes.Ldtoken, method.DeclaringType));
                body.Instructions.Add(OpCodes.Call.ToInstruction(getMethodFromHandleWithType));
            }
            else if (getMethodFromHandle != null)
            {
                body.Instructions.Add(OpCodes.Call.ToInstruction(getMethodFromHandle));
            }
            else
            {
                return false;
            }

            body.Instructions.Add(OpCodes.Ldstr.ToInstruction(moduleName));
            body.Instructions.Add(OpCodes.Ldstr.ToInstruction(methodName));
            EmitArgumentsArray(body, method);

            if (method.ReturnType.ElementType == ElementType.Void)
            {
                body.Instructions.Add(OpCodes.Call.ToInstruction(runVoidMethod));
            }
            else
            {
                var genericSig = new GenericInstMethodSig(method.ReturnType);
                var methodSpec = new MethodSpecUser((IMethodDefOrRef)runGenericDef, genericSig);
                body.Instructions.Add(OpCodes.Call.ToInstruction(methodSpec));
            }

            body.Instructions.Add(OpCodes.Ret.ToInstruction());
            method.Body = body;
            return true;
        }

        private static void EmitArgumentsArray(CilBody body, MethodDef method)
        {
            int parameterCount = method.Parameters.Count;
            if (parameterCount == 0)
            {
                body.Instructions.Add(OpCodes.Ldnull.ToInstruction());
                return;
            }

            body.Instructions.Add(OpCodes.Ldc_I4.ToInstruction(parameterCount));
            body.Instructions.Add(OpCodes.Newarr.ToInstruction(method.Module.CorLibTypes.Object.TypeDefOrRef));

            for (int i = 0; i < parameterCount; i++)
            {
                Parameter parameter = method.Parameters[i];
                TypeSig parameterType = parameter.Type;
                if (parameterType.IsByRef)
                {
                    throw new InvalidOperationException($"[LuaInvoke] does not support ref/out parameters in editor mode: {method.FullName}");
                }

                body.Instructions.Add(OpCodes.Dup.ToInstruction());
                body.Instructions.Add(OpCodes.Ldc_I4.ToInstruction(i));
                body.Instructions.Add(OpCodes.Ldarg.ToInstruction(parameter));

                if (parameterType.IsValueType)
                {
                    ITypeDefOrRef boxType = parameterType.ToTypeDefOrRef();
                    if (boxType == null)
                    {
                        throw new InvalidOperationException($"[LuaInvoke] cannot box parameter '{parameter.Name}' of method: {method.FullName}");
                    }

                    body.Instructions.Add(OpCodes.Box.ToInstruction(boxType));
                }

                body.Instructions.Add(OpCodes.Stelem_Ref.ToInstruction());
            }
        }

        private static void RewritePlayerMethod(ModuleDefMD module, MethodDef method, CustomAttribute luaInvokeAttr)
        {
            LuaInvokeWeaverShared.RemoveLuaInvokeAttribute(method, luaInvokeAttr);
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
