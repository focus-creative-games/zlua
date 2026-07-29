using dnlib.DotNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{
    public static class CodegenCommon
    {

        public static void AddCommonIncludes(CodeWriter writer)
        {
            writer.WriteLine("#include \"../lvm/LuaEnv.h\"");
            writer.WriteLine("#include \"../marshal/MarshalDefs.h\"");
            writer.WriteLine("#include \"../marshal/TypedMarshal.h\"");
            writer.WriteLine("#include \"../marshal/ArrayMarshal.h\"");
            writer.WriteLine("#include \"../marshal/ObjectMarshal.h\"");
            writer.WriteLine("#include \"../marshal/StringMarshal.h\"");
            writer.WriteLine("#include \"../marshal/StructMarshal.h\"");
            writer.WriteLine("#include \"../marshal/DelegateMarshal.h\"");
            writer.WriteLine("#include \"../marshal/MarshalMeta.h\"");
            writer.WriteLine("#include \"../marshal/OpaqueValueMarshal.h\"");
            writer.WriteLine("#include \"../utils/LuaException.h\"");
            writer.WriteLine("#include \"../utils/LuaUtil.h\"");
            writer.WriteLine("#include \"../utils/LuaStackGuard.h\"");
        }

        public static bool IsPassAsOpaqueValue(TypeSig paramType)
        {
            return paramType.IsByRef;
            // || paramType.ElementType == ElementType.Ptr || paramType.ElementType == ElementType.FnPtr;
        }

        public static string GeneratePushStatement(MethodDef methodDef, string metaExpr, string paramName, TypeSig paramType, LuaMarshalAsInfo marshalAsInfo)
        {
            string paramTypeName = NameUtil.GetTypeName(paramType);
            string typeExpr = $"{metaExpr}->type";
            string typeKlassExpr = $"{metaExpr}->typeKlass";
            // OpaqueValue always registers a stack-slot address (by-val: &value; byref: &ptrSlot).
            string addressExpr = $"&{paramName}";
            // string byTypeAddressExpr = param.IsByRef ? param.Name : $"&{param.Name}";

            LuaMarshalType marshalType = marshalAsInfo != null ? marshalAsInfo.marshalType : LuaMarshalType.Default;
            bool isArray = MetaUtil.IsSZOrMdArray(paramType);
            bool isString = MetaUtil.IsString(paramType);
            bool isByteArray = MetaUtil.IsByteArray(paramType);
            bool isPassAsOpaqueValue = IsPassAsOpaqueValue(paramType);
            bool isStruct = MetaUtil.IsStruct(paramType);
            bool isNullable = MetaUtil.IsNullable(paramType);
            bool isReferenceType = MetaUtil.IsReferenceType(paramType);
            bool isDelegate = MetaUtil.IsSubOfMulticastDelegate(paramType);
            bool isTypedReference = MetaUtil.IsTypedReference(paramType);
            string metatableRefExpr = $"MarshalMeta::EnsureByValMetatableRef(L, {metaExpr})";

        restart:
            switch (marshalType)
            {
            case LuaMarshalType.Default:
            {
                if (isArray)
                {
                    return $"ArrayMarshal::PushAsArrayObject(L, {paramName}, {metaExpr});";
                }
                if (isString)
                {
                    return $"StringMarshal::Push(L, {paramName});";
                }
                if (isDelegate)
                {
                    return $"DelegateMarshal::Push(L, reinterpret_cast<Il2CppDelegate*>({paramName}), {metatableRefExpr});";
                }
                if (isReferenceType)
                {
                    return $"ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>({paramName}), {metaExpr});";
                }
                if (isTypedReference)
                {
                    return $"OpaqueValueMarshal::PushTypedReference(L, {addressExpr});";
                }
                if (isPassAsOpaqueValue)
                {
                    return $"OpaqueValueMarshal::Push(L, {addressExpr}, {typeExpr});";
                }
                if (isStruct)
                {
                    if (isNullable)
                    {
                        return $"StructMarshal::PushNullableValue(L, {addressExpr}, {typeKlassExpr}, {metatableRefExpr});";
                    }
                    return $"StructMarshal::PushValue(L, {addressExpr}, {typeKlassExpr}, {metatableRefExpr});";
                }
                return $"DefaultTypedMarshal<{paramTypeName}>::Push(L, {paramName});";
            }
            case LuaMarshalType.UserData:
            {
                if (isReferenceType)
                {
                    return $"ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>({paramName}), {metaExpr});";
                }
                if (isStruct)
                {
                    if (isNullable)
                    {
                        return $"StructMarshal::PushNullableValue(L, {addressExpr}, {typeKlassExpr}, {metatableRefExpr});";
                    }
                    return $"StructMarshal::PushValue(L, {addressExpr}, {typeKlassExpr}, {metatableRefExpr});";
                }
                Debug.LogError(
                    $"only reference type or struct type is supported for marshal type {marshalType}, but got {paramType}, method: {methodDef.FullName}");
                marshalType = LuaMarshalType.Default;
                goto restart;
            }
            case LuaMarshalType.Bytes:
            {
                if (isString)
                {
                    return $"StringMarshal::Push(L, {paramName});";
                }
                if (isByteArray)
                {
                    return $"ArrayMarshal::PushAsBytes(L, {paramName});";
                }
                Debug.LogError(
                    $"only string or byte[] is supported for marshal type {marshalType}, but got {paramType}, method: {methodDef.FullName}");
                marshalType = LuaMarshalType.Default;
                goto restart;
            }
            case LuaMarshalType.OpaqueValue:
            {
                return $"OpaqueValueMarshal::Push(L, {addressExpr}, {typeExpr});";
            }
            case LuaMarshalType.Table:
            case LuaMarshalType.UnpackedValues:
                throw new CodeGenException(
                    $"composite marshal type {marshalType} uses DefaultInvoke (no stub); method: {methodDef.FullName}");
            default:
                throw new CodeGenException($"unsupported marshal type {marshalType}, method: {methodDef.FullName}");
            }
        }

        public static string GeneratePopStatement(MethodDef methodDef, string luaValueIndex, string metaExpr, string paramName,
         TypeSig paramType, LuaMarshalAsInfo marshalAsInfo)
        {
            string paramTypeName = NameUtil.GetTypeName(paramType);
            string typeExpr = $"{metaExpr}->type";
            string typeKlassExpr = $"{metaExpr}->typeKlass";
            bool isArray = MetaUtil.IsSZOrMdArray(paramType);
            bool isString = MetaUtil.IsString(paramType);
            bool isByteArray = MetaUtil.IsByteArray(paramType);
            bool isPassAsOpaqueValue = IsPassAsOpaqueValue(paramType);
            bool isStruct = MetaUtil.IsStruct(paramType);
            bool isNullable = MetaUtil.IsNullable(paramType);
            bool isReferenceType = MetaUtil.IsReferenceType(paramType);
            bool isTypedReference = MetaUtil.IsTypedReference(paramType);

            LuaMarshalType marshalType = marshalAsInfo != null ? marshalAsInfo.marshalType : LuaMarshalType.Default;
        restart:
            switch (marshalType)
            {
            case LuaMarshalType.Default:
            {
                if (isString)
                {
                    return $"{paramName} = StringMarshal::Pop(L, {luaValueIndex});";
                }
                if (isArray)
                {
                    return $"{paramName} = ArrayMarshal::PopFromArrayObjectOrTable(L, {luaValueIndex}, {typeKlassExpr});";
                }
                if (isReferenceType)
                {
                    return $"{paramName} = reinterpret_cast<{paramTypeName}>(ObjectMarshal::Pop(L, {luaValueIndex}, {typeKlassExpr}));";
                }
                if (isTypedReference)
                {
                    return $"OpaqueValueMarshal::PopTypedReference(L, {luaValueIndex}, &{paramName});";
                }
                if (isStruct)
                {
                    if (isNullable)
                    {
                        return $"StructMarshal::PopNullableValue(L, {luaValueIndex}, {typeKlassExpr}, &{paramName});";
                    }
                    return $"StructMarshal::PopValue(L, {luaValueIndex}, {typeKlassExpr}, &{paramName});";
                }
                if (isPassAsOpaqueValue)
                {
                    return $"OpaqueValueMarshal::Pop(L, -1, &{paramName}, {typeExpr});";
                }
                return $"{paramName} = DefaultTypedMarshal<{paramTypeName}>::Pop(L, {luaValueIndex});";
            }
            case LuaMarshalType.UserData:
            {
                if (isReferenceType)
                {
                    return $"{paramName} = reinterpret_cast<{paramTypeName}>(ObjectMarshal::Pop(L, {luaValueIndex}, {typeKlassExpr}));";
                }
                if (isStruct)
                {
                    if (isNullable)
                    {
                        return $"StructMarshal::PopNullableValue(L, {luaValueIndex}, {typeKlassExpr}, &{paramName});";
                    }
                    return $"StructMarshal::PopValue(L, {luaValueIndex}, {typeKlassExpr}, &{paramName});";
                }
                Debug.LogError(
                    $"only reference type or struct type is supported for marshal type {marshalType}, but got {paramType}, method: {methodDef.FullName}");
                marshalType = LuaMarshalType.Default;
                goto restart;
            }
            case LuaMarshalType.Bytes:
            {
                if (isString)
                {
                    return $"{paramName} = StringMarshal::Pop(L, {luaValueIndex});";
                }
                if (isByteArray)
                {
                    return $"{paramName} = ArrayMarshal::PopFromBytes(L, {luaValueIndex}, {typeKlassExpr});";
                }
                Debug.LogError(
                    $"only string or byte[] is supported for marshal type {marshalType}, but got {paramType}, method: {methodDef.FullName}");
                marshalType = LuaMarshalType.Default;
                goto restart;
            }
            case LuaMarshalType.OpaqueValue:
            {
                return $"OpaqueValueMarshal::Pop(L, {luaValueIndex}, &{paramName}, {typeExpr});";
            }
            case LuaMarshalType.Table:
            case LuaMarshalType.UnpackedValues:
                throw new CodeGenException(
                    $"composite marshal type {marshalType} uses DefaultInvoke (no stub); method: {methodDef.FullName}");
            default:
                throw new CodeGenException($"unsupported marshal type {marshalType}, method: {methodDef.FullName}");
            }
        }
    }
}
