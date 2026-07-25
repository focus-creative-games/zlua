using System;
using ZLua;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.DelegateImpl
{
    /// <summary>
    /// C#→Lua push/pop helpers used by Expression-compiled delegate bridges (GetFunction).
    /// </summary>
    internal static class CSharpToLuaMarshaling
    {
        internal static void PushByteArray(IntPtr L, byte[] value, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(L);
                    return;
                }

                ObjectMarshal.Push(L, value, typeof(byte[]));
                return;
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                BytesMarshal.PushByteArray(L, value);
                return;
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs push for byte[] ({marshalType})");
        }

        internal static void PushObject(IntPtr L, object value, Type declaredType, LuaMarshalType marshalType)
        {
            Type targetType = declaredType ?? value?.GetType() ?? typeof(object);
            if (marshalType == LuaMarshalType.Default)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(L);
                    return;
                }

                ObjectMarshal.Push(L, value, targetType);
                return;
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(L);
                    return;
                }

                ObjectMarshal.Push(L, value, targetType);
                return;
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs push for {targetType.Name} ({marshalType})");
        }

        internal static void PushString(IntPtr L, string value, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(L);
                }
                else
                {
                    LuaDll.lua_pushstring(L, value);
                }

                return;
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                BytesMarshal.PushStringAsOctets(L, value);
                return;
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(L);
                    return;
                }

                ObjectMarshal.Push(L, value, typeof(string));
                return;
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs push for string ({marshalType})");
        }

        internal static void PushStruct<TStruct>(IntPtr L, TStruct value, LuaMarshalType marshalType) where TStruct : struct
        {
            if (marshalType == LuaMarshalType.OpaqueValue)
            {
                object boxed = value;
                IntPtr handle = StructOpaqueScope.RegisterStruct(boxed, typeof(TStruct));
                LuaDll.lua_pushlightuserdata(L, handle);
                return;
            }

            if (marshalType == LuaMarshalType.Default)
            {
                throw new NotSupportedException(
                    $"struct {typeof(TStruct).Name} requires explicit LuaMarshalAs for C#→Lua push.");
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs push for {typeof(TStruct).Name} ({marshalType})");
        }

        internal static IntPtr PushByRefOpaque<T>(IntPtr L, ref T value) where T : struct
        {
            object boxed = value;
            IntPtr handle = StructOpaqueScope.RegisterStruct(boxed, typeof(T));
            LuaDll.lua_pushlightuserdata(L, handle);
            return handle;
        }

        internal static void WriteBackByRefOpaque<T>(IntPtr handle, ref T value) where T : struct
        {
            if (StructOpaqueScope.TryResolve(handle, typeof(T), out object updated) && updated is T typed)
            {
                value = typed;
            }
        }

        internal static byte[] PopByteArray(IntPtr L, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return (byte[])ObjectMarshal.Pop(L, index, typeof(byte[]));
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                return BytesMarshal.ReadByteArray(L, index);
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs pop for byte[] ({marshalType})");
        }

        internal static object PopObject(IntPtr L, int index, Type declaredType, LuaMarshalType marshalType)
        {
            Type targetType = declaredType ?? typeof(object);
            if (marshalType == LuaMarshalType.Default || marshalType == LuaMarshalType.UserData)
            {
                return ObjectMarshal.Pop(L, index, targetType);
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs pop for {targetType.Name} ({marshalType})");
        }

        internal static string PopString(IntPtr L, int index, LuaMarshalType marshalType)
        {
            if (marshalType == LuaMarshalType.Default)
            {
                return LuaDllExtension.tostring(L, index);
            }

            if (marshalType == LuaMarshalType.Bytes)
            {
                return BytesMarshal.ReadStringAsOctets(L, index);
            }

            if (marshalType == LuaMarshalType.UserData)
            {
                return (string)ObjectMarshal.Pop(L, index, typeof(string));
            }

            throw new NotSupportedException($"unsupported LuaMarshalAs pop for string ({marshalType})");
        }
    }
}
