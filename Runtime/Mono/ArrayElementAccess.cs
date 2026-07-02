using System;

namespace ZLua
{
    internal static class ArrayElementAccess
    {
        internal static int Get(IntPtr luaState)
        {
            try
            {
                if (!TryGetArray(luaState, 1, out Array array, out Type elementType))
                {
                    LuaCallbackBoundary.Throw("zlua: get expects array userdata");
                }

                int rank = array.Rank;
                int indexCount = LuaDll.lua_gettop(luaState) - 1;
                if (indexCount != rank)
                {
                    LuaCallbackBoundary.Throw($"zlua: get expects {rank} index argument(s)");
                }

                if (!TryReadIndices(luaState, 2, rank, out int[] indices))
                {
                    LuaCallbackBoundary.Throw("zlua: get expects integer indices");
                }

                object value = array.GetValue(indices);
                return TypeMethodRegistration.PushReturnDefaultPublic(luaState, elementType, value);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        internal static int Set(IntPtr luaState)
        {
            try
            {
                if (!TryGetArray(luaState, 1, out Array array, out Type elementType))
                {
                    LuaCallbackBoundary.Throw("zlua: set expects array userdata");
                }

                int rank = array.Rank;
                int argCount = LuaDll.lua_gettop(luaState) - 1;
                if (argCount != rank + 1)
                {
                    LuaCallbackBoundary.Throw($"zlua: set expects {rank} index argument(s) and a value");
                }

                if (!TryReadIndices(luaState, 2, rank, out int[] indices))
                {
                    LuaCallbackBoundary.Throw("zlua: set expects integer indices");
                }

                int valueIndex = LuaDll.lua_gettop(luaState);
                if (!TypeMethodRegistration.CanConvertArgumentValue(luaState, valueIndex, elementType))
                {
                    LuaCallbackBoundary.Throw("zlua: argument mismatch");
                }

                object value = TypeMethodRegistration.ReadValuePublic(luaState, valueIndex, elementType);
                value = ArrayMarshaling.CoerceToElementType(value, elementType);
                array.SetValue(value, indices);
                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        private static bool TryGetArray(IntPtr luaState, int index, out Array array, out Type elementType)
        {
            array = null;
            elementType = null;

            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, index, out object target) || target is not Array typedArray)
            {
                return false;
            }

            array = typedArray;
            elementType = array.GetType().GetElementType();
            return elementType != null;
        }

        private static bool TryReadIndices(IntPtr luaState, int startIndex, int rank, out int[] indices)
        {
            indices = new int[rank];
            for (int i = 0; i < rank; i++)
            {
                int luaIndex = startIndex + i;
                if (LuaDll.lua_type(luaState, luaIndex) != LuaDataType.Number)
                {
                    return false;
                }

                if (LuaDll.lua_isinteger(luaState, luaIndex) != 0)
                {
                    indices[i] = (int)LuaDll.lua_tointeger(luaState, luaIndex);
                    continue;
                }

                double number = LuaDll.lua_tonumber(luaState, luaIndex);
                if (Math.Truncate(number) != number)
                {
                    return false;
                }

                indices[i] = (int)number;
            }

            return true;
        }
    }
}
