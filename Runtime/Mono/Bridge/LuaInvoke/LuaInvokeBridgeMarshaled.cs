using System;
using ZLua.Marshaling;

namespace ZLua.Bridge
{
    public static partial class LuaInvokeBridge
    {
        public static byte[] InvokeM_BByteArray_BByteArray(int funcRef, byte[] value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushByteArray(scope.L, value, LuaMarshalType.Bytes);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopByteArray(scope.L, -1, LuaMarshalType.Bytes);
        }

        public static int InvokeM_Int32_BByteArray(int funcRef, byte[] value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushByteArray(scope.L, value, LuaMarshalType.Bytes);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.Default);
        }

        public static byte[] InvokeM_BByteArray(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopByteArray(scope.L, -1, LuaMarshalType.Bytes);
        }

        public static string InvokeM_BString_BString(int funcRef, string value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushString(scope.L, value, LuaMarshalType.Bytes);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopString(scope.L, -1, LuaMarshalType.Bytes);
        }

        public static string InvokeM_BString(int funcRef)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            if (!scope.TryPCall(0, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopString(scope.L, -1, LuaMarshalType.Bytes);
        }

        public static int InvokeM_Int32_OOpaque<TStruct>(int funcRef, TStruct value) where TStruct : struct
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushStruct(scope.L, value, LuaMarshalType.OpaqueLightUserData);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.Default);
        }

        /// <summary>Default OpaqueValue for <c>ref</c>/<c>in</c>/<c>out T</c> (any value type including primitives).</summary>
        public static int InvokeM_Int32_OByRef<T>(int funcRef, ref T value) where T : struct
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            IntPtr handle = LuaInvokeBridgeMarshaling.PushByRefOpaque(scope.L, ref value);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            LuaInvokeBridgeMarshaling.WriteBackByRefOpaque(handle, ref value);
            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.Default);
        }

        /// <summary><c>int F(ref int, int)</c> e.g. RoundtripOpaqueInt.</summary>
        public static int InvokeM_Int32_OByRef_Int32(int funcRef, ref int value, int arg1)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            IntPtr handle = LuaInvokeBridgeMarshaling.PushByRefOpaque(scope.L, ref value);
            LuaInvokeBridgeMarshaling.PushInt32(scope.L, arg1, LuaMarshalType.Default);
            if (!scope.TryPCall(2, 1))
            {
                return default;
            }

            LuaInvokeBridgeMarshaling.WriteBackByRefOpaque(handle, ref value);
            return LuaInvokeBridgeMarshaling.PopInt32(scope.L, -1, LuaMarshalType.Default);
        }

        public static string InvokeM_UString_UString(int funcRef, string value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushString(scope.L, value, LuaMarshalType.UserData);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopString(scope.L, -1, LuaMarshalType.UserData);
        }

        /// <summary>UserData string push → Lua reports type (Default string return).</summary>
        public static string InvokeM_String_UString(int funcRef, string value)
        {
            using CallScope scope = CallScope.Begin(funcRef);
            scope.PushFunction();
            LuaInvokeBridgeMarshaling.PushString(scope.L, value, LuaMarshalType.UserData);
            if (!scope.TryPCall(1, 1))
            {
                return default;
            }

            return LuaInvokeBridgeMarshaling.PopString(scope.L, -1, LuaMarshalType.Default);
        }
    }
}
