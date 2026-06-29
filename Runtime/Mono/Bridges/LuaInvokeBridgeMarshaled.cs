using System;

namespace ZLua
{
    internal static partial class LuaInvokeBridge
    {
        internal static byte[] InvokeM_BByteArray_BByteArray(int funcRef, byte[] value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushByteArray(scope.L, value, LuaMarshalType.Bytes);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopByteArray(scope.L, -1, LuaMarshalType.Bytes);
        }

        internal static int InvokeM_Int32_BByteArray(int funcRef, byte[] value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushByteArray(scope.L, value, LuaMarshalType.Bytes);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.Default);
        }

        internal static byte[] InvokeM_BByteArray(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return LuaInvokeBridgeMarshaling.PopByteArray(scope.L, -1, LuaMarshalType.Bytes);
        }

        internal static int InvokeM_UInt32_UInt32(int funcRef, int value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushInt32(scope.L, value, LuaMarshalType.UserData);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.UserData);
        }

        internal static string InvokeM_String_UInt32(int funcRef, int value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushInt32(scope.L, value, LuaMarshalType.UserData);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopString(scope.L, -1, LuaMarshalType.Default);
        }

        internal static TEnum InvokeM_UEnum<TEnum>(int funcRef) where TEnum : struct
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            scope.PCall(0, 1);
            return LuaInvokeBridgeMarshaling.PopEnum<TEnum>(scope.L, -1, LuaMarshalType.UserData);
        }

        internal static TEnum InvokeM_UEnum_UEnum<TEnum>(int funcRef, TEnum value) where TEnum : struct
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushEnum(scope.L, value, LuaMarshalType.UserData);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopEnum<TEnum>(scope.L, -1, LuaMarshalType.UserData);
        }

        internal static int InvokeM_Int32_OOpaque<TStruct>(int funcRef, TStruct value) where TStruct : struct
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushStruct(scope.L, value, LuaMarshalType.OpaqueLightUserData);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.Default);
        }

        internal static string InvokeM_UString_UString(int funcRef, string value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushString(scope.L, value, LuaMarshalType.UserData);
            scope.PCall(1, 1);
            return LuaInvokeBridgeMarshaling.PopString(scope.L, -1, LuaMarshalType.UserData);
        }
    }
}
