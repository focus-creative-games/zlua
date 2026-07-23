using System;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class ArrayMarshal
    {
        internal static bool TryGetConsecutiveTableLength(IntPtr L, int index, out int length)
        {
            length = 0;
            int absIndex = LuaDll.lua_absindex(L, index);
            if (LuaDll.lua_type(L, absIndex) != LuaDataType.Table)
            {
                return false;
            }

            for (int i = 1; ; i++)
            {
                LuaDll.lua_rawgeti(L, absIndex, i);
                if (LuaDll.lua_type(L, -1) == LuaDataType.Nil)
                {
                    LuaDll.lua_pop(L, 1);
                    break;
                }

                LuaDll.lua_pop(L, 1);
                length = i;
            }

            return true;
        }

        internal static Array PopSzArray(IntPtr L, int index, Type arrayType)
        {
            if (arrayType == null || !arrayType.IsArray || arrayType.GetArrayRank() != 1)
            {
                LuaCallbackBoundary.Throw("zlua internal error: PopSzArray expects szarray type");
            }

            LuaDataType valueType = LuaDll.lua_type(L, index);
            switch (valueType)
            {
                case LuaDataType.Nil:
                    return null;

                case LuaDataType.Table:
                    return PopFromTable(L, index, arrayType);

                case LuaDataType.UserData:
                    return PopFromArrayObject(L, index, arrayType);

                default:
                    LuaCallbackBoundary.Throw(
                        $"zlua argument mismatch: expected array or table, got: {valueType}");
                    return null;
            }
        }

        private static Array PopFromArrayObject(IntPtr L, int index, Type arrayType)
        {
            object obj = ObjectRegistry.Pop(L, index);
            if (obj == null)
            {
                return null;
            }

            if (!arrayType.IsInstanceOfType(obj))
            {
                LuaCallbackBoundary.Throw("zlua argument mismatch: array type mismatch");
            }

            return (Array)obj;
        }

        private static Array PopFromTable(IntPtr L, int index, Type arrayType)
        {
            if (!TryGetConsecutiveTableLength(L, index, out int length))
            {
                LuaCallbackBoundary.Throw("zlua argument mismatch: table value must be a table");
            }

            Type elementType = arrayType.GetElementType();
            Array newArray = Array.CreateInstance(elementType, length);
            int absIndex = LuaDll.lua_absindex(L, index);
            for (int i = 0; i < length; i++)
            {
                LuaDll.lua_rawgeti(L, absIndex, i + 1);
                object element = TypedMarshal.PopObject(L, -1, elementType);
                newArray.SetValue(element, i);
                LuaDll.lua_pop(L, 1);
            }

            return newArray;
        }

        /// <summary>
        /// Coerce Lua-numeric / boxed values into the array element type before <see cref="Array.SetValue"/>.
        /// </summary>
        internal static object CoerceToElementType(object value, Type elementType)
        {
            if (value == null || elementType == null || elementType.IsInstanceOfType(value))
            {
                return value;
            }

            Type target = Nullable.GetUnderlyingType(elementType) ?? elementType;
            if (target.IsEnum)
            {
                if (value is string s)
                {
                    return Enum.Parse(target, s);
                }

                return Enum.ToObject(target, Convert.ChangeType(value, Enum.GetUnderlyingType(target)));
            }

            if (target.IsPrimitive)
            {
                return Convert.ChangeType(value, target);
            }

            return value;
        }
    }
}
