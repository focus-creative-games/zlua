using System;
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Mt;
using ZLua.Utils;

namespace ZLua.Emit
{
    internal enum LuaOverloadMatch
    {
        None = 0,
        BestMatch = 1,
        Ambiguous = 2,
    }

    /// <summary>
    /// Non-throwing Lua→C# argument shape checks for same-arity overload selection.
    /// </summary>
    internal static class LuaArgMatcher
    {
        internal static LuaOverloadMatch Select(
            IntPtr L,
            int argStart,
            ConstructorInfo[] ctors,
            out int selectedIndex)
        {
            return SelectCore(L, argStart, ctors, out selectedIndex);
        }

        internal static LuaOverloadMatch SelectMethod(
            IntPtr L,
            int argStart,
            MethodInfo[] methods,
            out int selectedIndex)
        {
            return SelectCore(L, argStart, methods, out selectedIndex);
        }

        private static LuaOverloadMatch SelectCore(
            IntPtr L,
            int argStart,
            MethodBase[] candidates,
            out int selectedIndex)
        {
            selectedIndex = -1;
            int bestScore = -1;
            int bestOptionalUsed = int.MaxValue;
            int bestCount = 0;

            for (int i = 0; i < candidates.Length; i++)
            {
                if (!TryScore(L, argStart, candidates[i], out int score, out int optionalUsed))
                {
                    continue;
                }

                if (score > bestScore
                    || (score == bestScore && optionalUsed < bestOptionalUsed))
                {
                    bestScore = score;
                    bestOptionalUsed = optionalUsed;
                    selectedIndex = i;
                    bestCount = 1;
                }
                else if (score == bestScore && optionalUsed == bestOptionalUsed)
                {
                    bestCount++;
                }
            }

            if (bestCount == 0)
            {
                selectedIndex = -1;
                return LuaOverloadMatch.None;
            }

            if (bestCount > 1)
            {
                selectedIndex = -1;
                return LuaOverloadMatch.Ambiguous;
            }

            return LuaOverloadMatch.BestMatch;
        }

        private static bool TryScore(IntPtr L, int argStart, MethodBase method, out int score, out int optionalUsed)
        {
            score = 0;
            optionalUsed = 0;
            ParameterInfo[] parameters = method.GetParameters();
            int paramStart = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            int top = LuaDll.lua_gettop(L);
            int slot = argStart;
            for (int i = paramStart; i < parameters.Length; i++)
            {
                LuaMarshalBinding binding = LuaMarshalAsValidation.ResolveParameterBinding(
                    parameters[i],
                    method,
                    LuaMarshalDirection.LuaToCSharp);
                int need = binding.StackSlots > 0 ? binding.StackSlots : 1;
                int available = top >= slot ? top - slot + 1 : 0;
                if (available >= need)
                {
                    if (!TryMatchParameter(L, slot, parameters[i].ParameterType, binding, out int part))
                    {
                        score = -1;
                        return false;
                    }

                    score += part;
                    slot += need;
                    continue;
                }

                if (DefaultParameterUtil.TryGetDefaultValue(parameters[i], out _))
                {
                    optionalUsed++;
                    continue;
                }

                score = -1;
                return false;
            }

            return true;
        }

        private static bool TryMatchParameter(
            IntPtr L,
            int slot,
            Type declaredType,
            LuaMarshalBinding binding,
            out int score)
        {
            score = 0;
            if (binding != null && binding.MarshalType == LuaMarshalType.Table)
            {
                Type nullableUnderlying = Nullable.GetUnderlyingType(declaredType);
                if (nullableUnderlying != null && LuaDll.lua_isnil(L, slot))
                {
                    score = 1;
                    return true;
                }

                if (LuaDll.lua_istable(L, slot))
                {
                    score = 15;
                    return true;
                }

                return false;
            }

            if (binding != null && binding.MarshalType == LuaMarshalType.UnpackedValues)
            {
                for (int i = 0; i < binding.Members.Length; i++)
                {
                    Type memberType = CompositeMarshal.GetMemberType(binding.Members[i].Member);
                    if (!TryMatch(L, slot + i, memberType, out int part))
                    {
                        return false;
                    }

                    score += part;
                }

                return true;
            }

            return TryMatch(L, slot, declaredType, out score);
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

                // ByObj userdata with string view — e.g. zlua.cast(..., string). Use view, not runtime.
                if (luaType == LuaDataType.UserData
                    && ObjectRegistry.TryGetObject(L, index, out object strObj)
                    && strObj != null)
                {
                    Type viewType = ObjectRegistry.GetViewType(L, index) ?? strObj.GetType();
                    if (!typeof(string).IsAssignableFrom(viewType))
                    {
                        return false;
                    }

                    score = viewType == typeof(string) ? 20 : 10;
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

            if (declaredType.IsEnum)
            {
                if (luaType != LuaDataType.Number)
                {
                    return false;
                }

                // ImplicitEnum — below Identity / ImplicitNumeric for int32-range integers.
                score = 12;
                return true;
            }

            if (declaredType == typeof(decimal))
            {
                if (luaType != LuaDataType.Number)
                {
                    return false;
                }

                score = 10;
                return true;
            }

            if (declaredType.IsPrimitive && declaredType != typeof(IntPtr) && declaredType != typeof(UIntPtr))
            {
                if (luaType != LuaDataType.Number)
                {
                    return false;
                }

                // Align with Il2Cpp ConversionKind (METHOD_OVERLOAD §3.6):
                // I1–U4 + lua integer → Identity; I8/U8 → ImplicitExtendedInteger (worse).
                // R4/R8 + lua integer → ImplicitNumeric; otherwise Identity.
                bool isLuaInteger = LuaDll.lua_isinteger(L, index) != 0;
                if (declaredType == typeof(long) || declaredType == typeof(ulong))
                {
                    score = 8; // ImplicitExtendedInteger
                    return true;
                }

                if (declaredType == typeof(float) || declaredType == typeof(double))
                {
                    score = isLuaInteger ? 14 : 20; // ImplicitNumeric vs Identity
                    return true;
                }

                // bool already handled; remaining integral primitives (incl. char/byte/…).
                score = isLuaInteger ? 20 : 14; // Identity vs ImplicitNumeric
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
                    return false;
                }

                if (obj == null)
                {
                    score = 1;
                    return !declaredType.IsValueType;
                }

                // Overload / facade must use declared view, not runtime type (e.g. cast to object).
                Type viewType = ObjectRegistry.GetViewType(L, index) ?? obj.GetType();
                if (!declaredType.IsAssignableFrom(viewType))
                {
                    return false;
                }

                score = declaredType == viewType ? 20 : 10;
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
