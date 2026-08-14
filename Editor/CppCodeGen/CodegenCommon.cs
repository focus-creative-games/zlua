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
            writer.WriteLine("#include \"../marshal/PrimitiveMarshal.h\"");
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

        /// <summary>
        /// IntPtr/UIntPtr and other fixed-width primitives must not go through
        /// DefaultTypedMarshal&lt;T&gt; when a direct PrimitiveMarshal call is available
        /// (ABI aliases / hot-path specificity).
        /// </summary>
        public static string TryGeneratePrimitivePush(string paramTypeName, string paramName)
        {
            switch (paramTypeName)
            {
            case "bool":
                return $"PrimitiveMarshal::PushBool(L, {paramName});";
            case "int8_t":
                return $"PrimitiveMarshal::PushInt8(L, {paramName});";
            case "uint8_t":
                return $"PrimitiveMarshal::PushUInt8(L, {paramName});";
            case "int16_t":
                return $"PrimitiveMarshal::PushInt16(L, {paramName});";
            case "uint16_t":
                return $"PrimitiveMarshal::PushUInt16(L, {paramName});";
            case "int32_t":
                return $"PrimitiveMarshal::PushInt32(L, {paramName});";
            case "uint32_t":
                return $"PrimitiveMarshal::PushUInt32(L, {paramName});";
            case "int64_t":
                return $"PrimitiveMarshal::PushInt64(L, {paramName});";
            case "uint64_t":
                return $"PrimitiveMarshal::PushUInt64(L, {paramName});";
            case "float":
                return $"PrimitiveMarshal::PushFloat(L, {paramName});";
            case "double":
                return $"PrimitiveMarshal::PushDouble(L, {paramName});";
            case "intptr_t":
                return $"PrimitiveMarshal::PushIntPtr(L, {paramName});";
            case "uintptr_t":
                return $"PrimitiveMarshal::PushUIntPtr(L, {paramName});";
            default:
                return null;
            }
        }

        public static string TryGeneratePrimitivePop(string paramTypeName, string paramName, string luaValueIndex)
        {
            switch (paramTypeName)
            {
            case "bool":
                return $"{paramName} = PrimitiveMarshal::PopBool(L, {luaValueIndex});";
            case "int8_t":
                return $"{paramName} = PrimitiveMarshal::PopInt8(L, {luaValueIndex});";
            case "uint8_t":
                return $"{paramName} = PrimitiveMarshal::PopUInt8(L, {luaValueIndex});";
            case "int16_t":
                return $"{paramName} = PrimitiveMarshal::PopInt16(L, {luaValueIndex});";
            case "uint16_t":
                return $"{paramName} = PrimitiveMarshal::PopUInt16(L, {luaValueIndex});";
            case "int32_t":
                return $"{paramName} = PrimitiveMarshal::PopInt32(L, {luaValueIndex});";
            case "uint32_t":
                return $"{paramName} = PrimitiveMarshal::PopUInt32(L, {luaValueIndex});";
            case "int64_t":
                return $"{paramName} = PrimitiveMarshal::PopInt64(L, {luaValueIndex});";
            case "uint64_t":
                return $"{paramName} = PrimitiveMarshal::PopUInt64(L, {luaValueIndex});";
            case "float":
                return $"{paramName} = PrimitiveMarshal::PopFloat(L, {luaValueIndex});";
            case "double":
                return $"{paramName} = PrimitiveMarshal::PopDouble(L, {luaValueIndex});";
            case "intptr_t":
                return $"{paramName} = PrimitiveMarshal::PopIntPtr(L, {luaValueIndex});";
            case "uintptr_t":
                return $"{paramName} = PrimitiveMarshal::PopUIntPtr(L, {luaValueIndex});";
            default:
                return null;
            }
        }

        /// <summary>
        /// by-val primitive / IntPtr family / enum underlying — Default marshal only (no LuaMarshalAs override).
        /// </summary>
        public static bool IsDefaultPrimitiveLike(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            if (t.IsByRef)
            {
                return false;
            }

            switch (t.ElementType)
            {
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
            case ElementType.I8:
            case ElementType.U8:
            case ElementType.R4:
            case ElementType.R8:
            case ElementType.I:
            case ElementType.U:
                return true;
            case ElementType.ValueType:
            {
                TypeDef typeDef = t.ToTypeDefOrRef().ResolveTypeDef();
                return typeDef != null && typeDef.IsEnum;
            }
            default:
                return false;
            }
        }

        public static bool CanEmitDirectDefaultPrimitiveMarshal(TypeSig typeSig, LuaMarshalAsInfo marshalAsInfo)
        {
            if (!IsDefaultPrimitiveLike(typeSig))
            {
                return false;
            }

            if (marshalAsInfo != null && marshalAsInfo.marshalType != LuaMarshalType.Default)
            {
                return false;
            }

            return true;
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
                string primitivePush = TryGeneratePrimitivePush(paramTypeName, paramName);
                if (primitivePush != null)
                {
                    return primitivePush;
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
                string primitivePop = TryGeneratePrimitivePop(paramTypeName, paramName, luaValueIndex);
                if (primitivePop != null)
                {
                    return primitivePop;
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
