using System;
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Emit
{
    /// <summary>
    /// Non-throwing Lua→C# argument shape checks for same-arity overload selection.
    /// </summary>
    internal static class LuaArgMatcher
    {
        internal static bool TrySelect(
            IntPtr L,
            int argStart,
            ConstructorInfo[] ctors,
            out int selectedIndex)
        {
            selectedIndex = -1;
            int bestScore = -1;

            for (int i = 0; i < ctors.Length; i++)
            {
                ParameterInfo[] parameters = ctors[i].GetParameters();
                if (!TryScore(L, argStart, parameters, out int score))
                {
                    continue;
                }

                if (score > bestScore)
                {
                    bestScore = score;
                    selectedIndex = i;
                }
            }

            return selectedIndex >= 0;
        }

        private static bool TryScore(IntPtr L, int argStart, ParameterInfo[] parameters, out int score)
        {
            score = 0;
            for (int i = 0; i < parameters.Length; i++)
            {
                if (!TryMatch(L, argStart + i, parameters[i].ParameterType, out int part))
                {
                    score = -1;
                    return false;
                }

                score += part;
            }

            return true;
        }

        private static bool TryMatch(IntPtr L, int index, Type declaredType, out int score)
        {
            score = 0;
            if (declaredType == null)
            {
                return false;
            }

            if (declaredType.IsByRef)
            {
                declaredType = declaredType.GetElementType();
            }

            Type nullableUnderlying = Nullable.GetUnderlyingType(declaredType);
            if (nullableUnderlying != null)
            {
                if (LuaDll.lua_isnil(L, index))
                {
                    score = 1;
                    return true;
                }

                return TryMatch(L, index, nullableUnderlying, out score);
            }

            LuaDataType luaType = LuaDll.lua_type(L, index);

            if (declaredType == typeof(string))
            {
                if (luaType == LuaDataType.String)
                {
                    score = 10;
                    return true;
                }

                if (luaType == LuaDataType.Nil)
                {
                    score = 1;
                    return true;
                }

                return false;
            }

            if (declaredType == typeof(bool))
            {
                if (luaType != LuaDataType.Boolean)
                {
                    return false;
                }

                score = 10;
                return true;
            }

            if (declaredType.IsEnum
                || (declaredType.IsPrimitive && declaredType != typeof(IntPtr) && declaredType != typeof(UIntPtr))
                || declaredType == typeof(decimal))
            {
                if (luaType != LuaDataType.Number)
                {
                    return false;
                }

                score = 10;
                return true;
            }

            if (declaredType == typeof(IntPtr) || declaredType == typeof(UIntPtr)
                || PointerMarshal.IsPointerLikeType(declaredType))
            {
                if (luaType == LuaDataType.Number || luaType == LuaDataType.LightUserData
                    || luaType == LuaDataType.UserData || luaType == LuaDataType.Nil)
                {
                    score = 5;
                    return true;
                }

                return false;
            }

            if (StructMarshal.IsStructType(declaredType))
            {
                if (luaType != LuaDataType.UserData)
                {
                    return false;
                }

                unsafe
                {
                    ByValUserDataHeader* header = StructMarshal.GetByValHeader(L, index);
                    if (header == null)
                    {
                        return false;
                    }

                    Type headerType = TypeHandleStore.GetType(header->TypeHandle);
                    if (headerType != declaredType)
                    {
                        return false;
                    }
                }

                score = 20;
                return true;
            }

            if (typeof(Delegate).IsAssignableFrom(declaredType))
            {
                if (luaType == LuaDataType.Function)
                {
                    score = 10;
                    return true;
                }

                if (luaType == LuaDataType.Nil)
                {
                    score = 1;
                    return true;
                }

                if (luaType == LuaDataType.UserData
                    && ObjectRegistry.TryGetObject(L, index, out object delObj)
                    && delObj != null
                    && declaredType.IsInstanceOfType(delObj))
                {
                    score = 20;
                    return true;
                }

                return false;
            }

            if (declaredType.IsArray && declaredType.GetArrayRank() == 1)
            {
                if (luaType == LuaDataType.Table)
                {
                    score = 5;
                    return true;
                }

                if (luaType == LuaDataType.Nil)
                {
                    score = 1;
                    return true;
                }

                if (luaType == LuaDataType.UserData
                    && ObjectRegistry.TryGetObject(L, index, out object arr)
                    && arr != null
                    && declaredType.IsInstanceOfType(arr))
                {
                    score = 20;
                    return true;
                }

                return false;
            }

            // Reference / object facade.
            if (luaType == LuaDataType.Nil)
            {
                score = declaredType == typeof(object) ? 1 : 2;
                return !declaredType.IsValueType;
            }

            if (luaType == LuaDataType.String)
            {
                if (!declaredType.IsAssignableFrom(typeof(string)))
                {
                    return false;
                }

                score = declaredType == typeof(string) ? 10 : 3;
                return true;
            }

            if (luaType == LuaDataType.Boolean)
            {
                if (!declaredType.IsAssignableFrom(typeof(bool)))
                {
                    return false;
                }

                score = declaredType == typeof(bool) ? 10 : 3;
                return true;
            }

            if (luaType == LuaDataType.Number)
            {
                if (declaredType.IsAssignableFrom(typeof(int)) || declaredType.IsAssignableFrom(typeof(double)))
                {
                    score = 3;
                    return true;
                }

                return false;
            }

            if (luaType == LuaDataType.Function)
            {
                if (!typeof(Delegate).IsAssignableFrom(declaredType))
                {
                    return false;
                }

                score = 10;
                return true;
            }

            if (luaType == LuaDataType.Table)
            {
                if (!declaredType.IsArray)
                {
                    return false;
                }

                score = 5;
                return true;
            }

            if (luaType == LuaDataType.UserData)
            {
                if (!ObjectRegistry.TryGetObject(L, index, out object obj))
                {
                    // ByVal userdata into a reference parameter — usually not a match.
                    return false;
                }

                if (obj == null)
                {
                    score = 1;
                    return !declaredType.IsValueType;
                }

                if (!declaredType.IsInstanceOfType(obj))
                {
                    return false;
                }

                score = declaredType == obj.GetType() ? 20 : 10;
                if (declaredType == typeof(object))
                {
                    score = 2;
                }

                return true;
            }

            return false;
        }
    }
}
