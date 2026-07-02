using System;
using ZLua;
using ZLua.Marshal;
using ZLua.Mt;
using ZLua.DelegateImpl;

namespace ZLua.MethodBridge
{
    /// <summary>
    /// Typed push/pop for woven <see cref="LuaInvoke"/> (no MethodInfo / object[]).
    /// </summary>
    internal static class LuaInvokeBridgeMarshaling
    {
        internal static void PushInt32(IntPtr luaState, int value, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                LuaDll.lua_pushinteger(luaState, value);
                return;
            }

            ValueMarshaling.PushArgument(luaState, value, typeof(int), marshalType);
        }

        internal static void PushByteArray(IntPtr luaState, byte[] value, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                // MARSHAL_SPEC §1.2: Default byte[] → ByObjUserData.
                ValueMarshaling.PushArgument(luaState, value, typeof(byte[]), LuaMarshalType.UserData);
                return;
            }

            ValueMarshaling.PushArgument(luaState, value, typeof(byte[]), marshalType);
        }

        internal static void PushObject(IntPtr luaState, object value, Type declaredType, LuaMarshalType marshalType)
        {
            Type targetType = declaredType ?? value?.GetType() ?? typeof(object);
            if (marshalType == LuaMarshalType.Default)
            {
                ValueMarshaling.PushArgument(luaState, value, targetType, LuaMarshalType.UserData);
                return;
            }

            ValueMarshaling.PushArgument(luaState, value, targetType, marshalType);
        }

        internal static void PushString(IntPtr luaState, string value, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(luaState);
                }
                else
                {
                    LuaDll.lua_pushstring(luaState, value);
                }

                return;
            }

            ValueMarshaling.PushArgument(luaState, value, typeof(string), marshalType);
        }

        internal static void PushEnum<TEnum>(IntPtr luaState, TEnum value, LuaMarshalType marshalType) where TEnum : struct
        {
            if (marshalType == LuaMarshalType.Default)
            {
                LuaDll.lua_pushinteger(luaState, Convert.ToInt32(value));
                return;
            }

            ValueMarshaling.PushArgument(luaState, value, typeof(TEnum), marshalType);
        }

        internal static void PushStruct<TStruct>(IntPtr luaState, TStruct value, LuaMarshalType marshalType) where TStruct : struct
        {
            if (marshalType == LuaMarshalType.OpaqueLightUserData)
            {
                ValueMarshaling.PushArgument(luaState, value, typeof(TStruct), marshalType);
                return;
            }

            if (marshalType == LuaMarshalType.Default)
            {
                throw new NotSupportedException($"struct {typeof(TStruct).Name} requires explicit LuaMarshalAs for LuaInvoke push.");
            }

            ValueMarshaling.PushArgument(luaState, value, typeof(TStruct), marshalType);
        }

        /// <summary>
        /// Push ref/in/out as OpaqueValue (MARSHAL_SPEC §4.3) and return handle for write-back after pcall.
        /// </summary>
        internal static IntPtr PushByRefOpaque<T>(IntPtr luaState, ref T value) where T : struct
        {
            object boxed = value;
            IntPtr handle = StructOpaqueScope.RegisterStruct(boxed, typeof(T));
            PointerMarshaling.PushPointer(luaState, handle);
            return handle;
        }

        internal static void WriteBackByRefOpaque<T>(IntPtr handle, ref T value) where T : struct
        {
            if (StructOpaqueScope.TryResolve(handle, typeof(T), out object updated) && updated is T typed)
            {
                value = typed;
            }
        }

        internal static int PopInt32(IntPtr luaState, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return (int)LuaDll.lua_tointeger(luaState, index);
            }

            return (int)ValueMarshaling.Read(luaState, index, typeof(int), marshalType);
        }

        internal static byte[] PopByteArray(IntPtr luaState, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return (byte[])ValueMarshaling.Read(luaState, index, typeof(byte[]), LuaMarshalType.UserData);
            }

            return (byte[])ValueMarshaling.Read(luaState, index, typeof(byte[]), marshalType);
        }

        internal static object PopObject(IntPtr luaState, int index, Type declaredType, LuaMarshalType marshalType)
        {
            Type targetType = declaredType ?? typeof(object);
            if (marshalType == LuaMarshalType.Default)
            {
                return ValueMarshaling.Read(luaState, index, targetType, LuaMarshalType.UserData);
            }

            return ValueMarshaling.Read(luaState, index, targetType, marshalType);
        }

        internal static string PopString(IntPtr luaState, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return LuaDllExtension.tostring(luaState, index);
            }

            return (string)ValueMarshaling.Read(luaState, index, typeof(string), marshalType);
        }

        internal static TEnum PopEnum<TEnum>(IntPtr luaState, int index, LuaMarshalType marshalType) where TEnum : struct
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return (TEnum)Enum.ToObject(typeof(TEnum), (int)LuaDll.lua_tointeger(luaState, index));
            }

            return (TEnum)ValueMarshaling.Read(luaState, index, typeof(TEnum), marshalType);
        }
    }
}
