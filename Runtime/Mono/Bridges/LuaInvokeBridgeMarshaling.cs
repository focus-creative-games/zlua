using System;

namespace ZLua
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

            LuaMarshalDispatch.PushArgument(luaState, value, typeof(int), marshalType);
        }

        internal static void PushByteArray(IntPtr luaState, byte[] value, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                throw new NotSupportedException("byte[] requires LuaMarshalType.Bytes for LuaInvoke push.");
            }

            LuaMarshalDispatch.PushArgument(luaState, value, typeof(byte[]), marshalType);
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

            LuaMarshalDispatch.PushArgument(luaState, value, typeof(string), marshalType);
        }

        internal static void PushEnum<TEnum>(IntPtr luaState, TEnum value, LuaMarshalType marshalType) where TEnum : struct
        {
            if (marshalType == LuaMarshalType.Default)
            {
                LuaDll.lua_pushinteger(luaState, Convert.ToInt32(value));
                return;
            }

            LuaMarshalDispatch.PushArgument(luaState, value, typeof(TEnum), marshalType);
        }

        internal static void PushStruct<TStruct>(IntPtr luaState, TStruct value, LuaMarshalType marshalType) where TStruct : struct
        {
            if (marshalType == LuaMarshalType.OpaqueLightUserData)
            {
                LuaMarshalDispatch.PushArgument(luaState, value, typeof(TStruct), marshalType);
                return;
            }

            if (marshalType == LuaMarshalType.Default)
            {
                throw new NotSupportedException($"struct {typeof(TStruct).Name} requires explicit LuaMarshalAs for LuaInvoke push.");
            }

            LuaMarshalDispatch.PushArgument(luaState, value, typeof(TStruct), marshalType);
        }

        internal static int PopInt32(IntPtr luaState, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return (int)LuaDll.lua_tointeger(luaState, index);
            }

            return (int)LuaMarshalDispatch.Read(luaState, index, typeof(int), marshalType);
        }

        internal static byte[] PopByteArray(IntPtr luaState, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                throw new NotSupportedException("byte[] return requires LuaMarshalType.Bytes for LuaInvoke pop.");
            }

            return (byte[])LuaMarshalDispatch.Read(luaState, index, typeof(byte[]), marshalType);
        }

        internal static string PopString(IntPtr luaState, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return LuaDllExtension.tostring(luaState, index);
            }

            return (string)LuaMarshalDispatch.Read(luaState, index, typeof(string), marshalType);
        }

        internal static TEnum PopEnum<TEnum>(IntPtr luaState, int index, LuaMarshalType marshalType) where TEnum : struct
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return (TEnum)Enum.ToObject(typeof(TEnum), (int)LuaDll.lua_tointeger(luaState, index));
            }

            return (TEnum)LuaMarshalDispatch.Read(luaState, index, typeof(TEnum), marshalType);
        }
    }
}
