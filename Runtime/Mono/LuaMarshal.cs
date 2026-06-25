using System;

namespace ZLua
{
    public static class LuaMarshal
    {
        public static void PushObject(IntPtr luaState, object value)
        {
            if (value == null)
            {
                throw new NotSupportedException("nil marshal is not implemented in v0.0.1.");
            }

            Type t = value.GetType();
            if (t == typeof(int))
            {
                LuaDll.lua_pushinteger(luaState, (int)value);
                return;
            }

            if (t == typeof(long))
            {
                LuaDll.lua_pushinteger(luaState, (long)value);
                return;
            }

            if (t == typeof(float))
            {
                LuaDll.lua_pushnumber(luaState, (float)value);
                return;
            }

            if (t == typeof(double))
            {
                LuaDll.lua_pushnumber(luaState, (double)value);
                return;
            }

            if (t == typeof(bool))
            {
                LuaDll.lua_pushboolean(luaState, (bool)value ? 1 : 0);
                return;
            }

            if (t == typeof(string))
            {
                LuaDll.lua_pushstring(luaState, (string)value);
                return;
            }

            throw new NotSupportedException($"Type {t.FullName} is not supported by LuaMarshal.PushObject.");
        }

        public static object PopObject(IntPtr luaState, Type targetType, int index = -1)
        {
            if (targetType == typeof(void))
            {
                return null;
            }

            if (targetType == typeof(int))
            {
                return (int)LuaDll.lua_tointeger(luaState, index);
            }

            if (targetType == typeof(long))
            {
                return LuaDll.lua_tointeger(luaState, index);
            }

            if (targetType == typeof(float))
            {
                return (float)LuaDll.lua_tonumber(luaState, index);
            }

            if (targetType == typeof(double))
            {
                return LuaDll.lua_tonumber(luaState, index);
            }

            if (targetType == typeof(bool))
            {
                return LuaDll.lua_toboolean(luaState, index) != 0;
            }

            if (targetType == typeof(string))
            {
                return LuaDllExtension.tostring(luaState, index);
            }

            throw new NotSupportedException($"Type {targetType.FullName} is not supported by LuaMarshal.PopObject.");
        }

        public static void PushAny<T>(IntPtr luaState, T value)
        {
            if (value is int i)
            {
                LuaDll.lua_pushinteger(luaState, i);
                return;
            }

            if (value is long l)
            {
                LuaDll.lua_pushinteger(luaState, l);
                return;
            }

            if (value is float f)
            {
                LuaDll.lua_pushnumber(luaState, f);
                return;
            }

            if (value is double d)
            {
                LuaDll.lua_pushnumber(luaState, d);
                return;
            }

            if (value is bool b)
            {
                LuaDll.lua_pushboolean(luaState, b ? 1 : 0);
                return;
            }

            if (value is string s)
            {
                LuaDll.lua_pushstring(luaState, s);
                return;
            }

            throw new NotSupportedException($"Type {typeof(T).FullName} is not supported by LuaMarshal.PushAny.");
        }

        public static T PopAny<T>(IntPtr luaState)
        {
            Type t = typeof(T);
            object value;
            if (t == typeof(int))
            {
                value = (int)LuaDll.lua_tointeger(luaState, -1);
            }
            else if (t == typeof(long))
            {
                value = LuaDll.lua_tointeger(luaState, -1);
            }
            else if (t == typeof(float))
            {
                value = (float)LuaDll.lua_tonumber(luaState, -1);
            }
            else if (t == typeof(double))
            {
                value = LuaDll.lua_tonumber(luaState, -1);
            }
            else if (t == typeof(bool))
            {
                value = LuaDll.lua_toboolean(luaState, -1) != 0;
            }
            else if (t == typeof(string))
            {
                value = LuaDllExtension.tostring(luaState, -1);
            }
            else
            {
                throw new NotSupportedException($"Type {t.FullName} is not supported by LuaMarshal.PopAny.");
            }

            return (T)value;
        }
    }
}
