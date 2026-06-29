using System;

namespace ZLua
{
    public static class LuaMarshal
    {
        public static void PushObject(IntPtr luaState, object value)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            Type runtimeType = value.GetType();
            try
            {
                TypeMethodRegistration.PushArgumentValue(luaState, value, runtimeType);
            }
            catch (NotSupportedException)
            {
                if (runtimeType.IsArray || runtimeType.IsClass || runtimeType.IsInterface)
                {
                    LuaManagerObject.PushConstructorInstance(luaState, value, runtimeType);
                    return;
                }

                if (ValueTypeMarshaling.IsStructType(runtimeType))
                {
                    LuaManagerObject.PushConstructorInstance(luaState, value, runtimeType);
                    return;
                }

                if (typeof(Delegate).IsAssignableFrom(runtimeType))
                {
                    LuaManagerObject.PushConstructorInstance(luaState, value, runtimeType);
                    return;
                }

                throw;
            }
        }

        public static object PopObject(IntPtr luaState, Type targetType, int index = -1)
        {
            if (targetType == typeof(void))
            {
                return null;
            }

            return TypeMethodRegistration.ReadArgumentValue(luaState, index, targetType);
        }

        public static void PushAny<T>(IntPtr luaState, T value)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            PushObject(luaState, value);
        }

        public static T PopAny<T>(IntPtr luaState)
        {
            object value = PopObject(luaState, typeof(T), -1);
            return (T)value;
        }
    }
}
