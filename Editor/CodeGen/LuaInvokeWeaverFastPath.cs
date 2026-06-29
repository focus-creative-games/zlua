using System;
using System.Collections.Generic;
using dnlib.DotNet;
using dnlib.DotNet.Emit;
using DnlibFieldAttributes = dnlib.DotNet.FieldAttributes;
using DnlibTypeAttributes = dnlib.DotNet.TypeAttributes;
using MethodAttributes = dnlib.DotNet.MethodAttributes;
using MethodImplAttributes = dnlib.DotNet.MethodImplAttributes;

namespace ZLua
{
    internal static class LuaInvokeWeaverFastPath
    {
        internal const string SitesNestedTypeName = "__ZLuaInvokeSites";
        internal const string SitesFieldPrefix = "_funcRef_";

        internal static bool TryRewriteEditorMethod(
            ModuleDefMD module,
            MethodDef method,
            CustomAttribute luaInvokeAttr,
            string moduleName,
            string methodName,
            IMethod bridgeMethod,
            IMethod getOrCreateFunctionRefMethod)
        {
            if (bridgeMethod == null || getOrCreateFunctionRefMethod == null)
            {
                return false;
            }

            FieldDef siteField = EnsureSiteField(module, method, moduleName, methodName, getOrCreateFunctionRefMethod);
            if (luaInvokeAttr != null)
            {
                LuaInvokeWeaverShared.RemoveLuaInvokeAttribute(method, luaInvokeAttr);
            }

            method.ImplMap = null;
            method.Attributes &= ~MethodAttributes.PinvokeImpl;
            method.ImplAttributes &= ~MethodImplAttributes.InternalCall;
            method.ImplAttributes &= ~MethodImplAttributes.Native;
            method.ImplAttributes |= MethodImplAttributes.Managed | MethodImplAttributes.IL;

            var body = new CilBody { InitLocals = false };
            body.Instructions.Add(OpCodes.Ldsfld.ToInstruction(siteField));

            for (int i = 0; i < method.Parameters.Count; i++)
            {
                body.Instructions.Add(OpCodes.Ldarg.ToInstruction(method.Parameters[i]));
            }

            body.Instructions.Add(OpCodes.Call.ToInstruction(bridgeMethod));
            body.Instructions.Add(OpCodes.Ret.ToInstruction());
            method.Body = body;
            return true;
        }

        private static FieldDef EnsureSiteField(
            ModuleDefMD module,
            MethodDef method,
            string moduleName,
            string luaMethodName,
            IMethod getOrCreateFunctionRefMethod)
        {
            TypeDef declaringType = method.DeclaringType
                ?? throw new InvalidOperationException($"[LuaInvoke] missing declaring type: {method.FullName}");

            TypeDef sitesType = GetOrCreateSitesType(module, declaringType);
            string fieldName = SitesFieldPrefix + method.Name;
            FieldDef existing = sitesType.FindField(fieldName);
            if (existing != null)
            {
                return existing;
            }

            var field = new FieldDefUser(
                fieldName,
                new FieldSig(module.CorLibTypes.Int32),
                DnlibFieldAttributes.Assembly | DnlibFieldAttributes.Static | DnlibFieldAttributes.Private);
            sitesType.Fields.Add(field);
            AppendSiteInitializer(module, sitesType, field, moduleName, luaMethodName, getOrCreateFunctionRefMethod);
            return field;
        }

        private static TypeDef GetOrCreateSitesType(ModuleDefMD module, TypeDef declaringType)
        {
            foreach (TypeDef nested in declaringType.NestedTypes)
            {
                if (string.Equals(nested.Name, SitesNestedTypeName, StringComparison.Ordinal))
                {
                    return nested;
                }
            }

            var sitesType = new TypeDefUser(
                null,
                SitesNestedTypeName,
                module.CorLibTypes.Object.TypeDefOrRef);
            sitesType.Attributes = DnlibTypeAttributes.NestedAssembly
                | DnlibTypeAttributes.Sealed
                | DnlibTypeAttributes.Abstract
                | DnlibTypeAttributes.BeforeFieldInit;
            sitesType.DeclaringType = declaringType;
            return sitesType;
        }

        private static void AppendSiteInitializer(
            ModuleDefMD module,
            TypeDef sitesType,
            FieldDef field,
            string moduleName,
            string luaMethodName,
            IMethod getOrCreateFunctionRefMethod)
        {
            MethodDef cctor = FindOrCreateStaticConstructor(sitesType, module);
            IList<Instruction> instructions = cctor.Body.Instructions;
            int insertIndex = instructions.Count;
            if (insertIndex > 0 && instructions[insertIndex - 1].OpCode == OpCodes.Ret)
            {
                insertIndex -= 1;
            }

            instructions.Insert(insertIndex++, OpCodes.Ldstr.ToInstruction(moduleName));
            instructions.Insert(insertIndex++, OpCodes.Ldstr.ToInstruction(luaMethodName));
            instructions.Insert(insertIndex++, OpCodes.Call.ToInstruction(getOrCreateFunctionRefMethod));
            instructions.Insert(insertIndex, OpCodes.Stsfld.ToInstruction(field));
        }

        private static MethodDef FindOrCreateStaticConstructor(TypeDef type, ModuleDef module)
        {
            for (int i = 0; i < type.Methods.Count; i++)
            {
                if (type.Methods[i].IsStaticConstructor)
                {
                    MethodDef existing = type.Methods[i];
                    if (existing.Body == null)
                    {
                        existing.Body = new CilBody();
                        existing.Body.Instructions.Add(OpCodes.Ret.ToInstruction());
                    }

                    return existing;
                }
            }

            var cctor = new MethodDefUser(
                ".cctor",
                MethodSig.CreateStatic(module.CorLibTypes.Void),
                MethodImplAttributes.IL | MethodImplAttributes.Managed,
                MethodAttributes.Private
                    | MethodAttributes.Static
                    | MethodAttributes.HideBySig
                    | MethodAttributes.SpecialName
                    | MethodAttributes.RTSpecialName);
            cctor.Body = new CilBody();
            cctor.Body.Instructions.Add(OpCodes.Ret.ToInstruction());
            type.Methods.Add(cctor);
            return cctor;
        }
    }
}
