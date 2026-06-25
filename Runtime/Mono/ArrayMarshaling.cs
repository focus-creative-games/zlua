using System;

namespace ZLua
{
    internal static class ArrayMarshaling
    {
        internal static bool IsSzArrayType(Type type)
        {
            return type != null && type.IsArray && type.GetArrayRank() == 1;
        }

        internal static bool IsMdArrayType(Type type)
        {
            return type != null && type.IsArray;
        }

        internal static Array CreateSzArray(Type elementType, int length)
        {
            return Array.CreateInstance(elementType, length);
        }

        internal static Array CreateMdArray(Type elementType, int[] sizes, int[] lowerBounds)
        {
            return Array.CreateInstance(elementType, sizes, lowerBounds);
        }

        internal static bool TryReadIntSequence(
            IntPtr luaState,
            int index,
            int expectedCount,
            out int[] values,
            out string error)
        {
            values = null;
            error = null;

            int absIndex = LuaDll.lua_absindex(luaState, index);
            if (LuaDll.lua_type(luaState, absIndex) != LuaDataType.Table)
            {
                error = "expected table";
                return false;
            }

            if (expectedCount < 0)
            {
                error = "invalid expected count";
                return false;
            }

            values = new int[expectedCount];
            for (int i = 0; i < expectedCount; i++)
            {
                LuaDll.lua_pushinteger(luaState, i + 1);
                LuaDataType valueType = LuaDll.lua_rawget(luaState, absIndex);
                if (valueType != LuaDataType.Number || LuaDll.lua_isinteger(luaState, -1) == 0)
                {
                    LuaDll.lua_pop(luaState, 1);
                    error = $"table index {i + 1} must be integer";
                    return false;
                }

                values[i] = (int)LuaDll.lua_tointeger(luaState, -1);
                LuaDll.lua_pop(luaState, 1);
            }

            return true;
        }

        internal static bool TryGetConsecutiveTableLength(IntPtr luaState, int index, out int length, out string error)
        {
            length = 0;
            error = null;

            int absIndex = LuaDll.lua_absindex(luaState, index);
            if (LuaDll.lua_type(luaState, absIndex) != LuaDataType.Table)
            {
                error = "expected table";
                return false;
            }

            for (int i = 1; ; i++)
            {
                LuaDll.lua_pushinteger(luaState, i);
                LuaDataType valueType = LuaDll.lua_rawget(luaState, absIndex);
                if (valueType == LuaDataType.Nil)
                {
                    LuaDll.lua_pop(luaState, 1);
                    break;
                }

                LuaDll.lua_pop(luaState, 1);
                length = i;
            }

            return true;
        }
    }
}
