// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.Reflection;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    /// <summary>
    /// Table / UnpackedValues pop-push using pre-resolved Members (no name reflection on hot path).
    /// </summary>
    internal static class CompositeMarshal
    {
        internal static object Pop(IntPtr L, int valueIdx, Type declaredType, LuaMarshalBinding binding)
        {
            if (binding == null || !binding.IsComposite)
            {
                return TypedMarshal.PopObject(L, valueIdx, declaredType);
            }

            if (CompositeSpecialized.TryPop(L, valueIdx, declaredType, binding, out object specialized))
            {
                return specialized;
            }

            return binding.MarshalType == LuaMarshalType.Table
                ? PopTable(L, valueIdx, declaredType, binding.Members)
                : PopUnpacked(L, valueIdx, declaredType, binding.Members);
        }

        internal static int Push(IntPtr L, object value, Type declaredType, LuaMarshalBinding binding)
        {
            if (declaredType == typeof(void))
            {
                return 0;
            }

            if (binding == null || !binding.IsComposite)
            {
                TypedMarshal.PushObject(L, value, declaredType);
                return 1;
            }

            if (CompositeSpecialized.TryPush(L, value, declaredType, binding, out int specializedSlots))
            {
                return specializedSlots;
            }

            if (binding.MarshalType == LuaMarshalType.Table)
            {
                PushTable(L, value, declaredType, binding.Members);
                return 1;
            }

            PushUnpacked(L, value, declaredType, binding.Members);
            return binding.StackSlots;
        }

        internal static object PopTable(IntPtr L, int index, Type declaredType, LuaMarshalMemberBinding[] members)
        {
            Type nullableUnderlying = Nullable.GetUnderlyingType(declaredType);
            if (nullableUnderlying != null)
            {
                if (LuaDll.lua_isnil(L, index))
                {
                    return null;
                }

                object inner = PopTable(L, index, nullableUnderlying, members);
                return Activator.CreateInstance(declaredType, inner);
            }

            if (!LuaDll.lua_istable(L, index))
            {
                LuaCallbackBoundary.Throw(
                    $"zlua: expected table for LuaMarshalType.Table ({declaredType.FullName}), got {LuaDll.lua_type(L, index)}");
            }

            object instance = Activator.CreateInstance(declaredType);
            for (int i = 0; i < members.Length; i++)
            {
                LuaMarshalMemberBinding entry = members[i];
                LuaDataType fieldType = LuaDll.lua_getfield(L, index, entry.ClrName);
                try
                {
                    if (fieldType == LuaDataType.Nil)
                    {
                        if (entry.IsOptional)
                        {
                            continue;
                        }

                        LuaCallbackBoundary.Throw(
                            $"zlua: missing table key '{entry.ClrName}' for {declaredType.FullName}");
                    }

                    Type memberType = GetMemberType(entry.Member);
                    object memberValue = TypedMarshal.PopObject(L, -1, memberType);
                    SetMember(instance, entry.Member, memberValue);
                }
                finally
                {
                    LuaDll.lua_pop(L, 1);
                }
            }

            return instance;
        }

        internal static void PushTable(IntPtr L, object value, Type declaredType, LuaMarshalMemberBinding[] members)
        {
            Type nullableUnderlying = Nullable.GetUnderlyingType(declaredType);
            if (nullableUnderlying != null)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(L);
                    return;
                }

                object inner = value;
                Type valueType = value.GetType();
                if (valueType.IsGenericType && valueType.GetGenericTypeDefinition() == typeof(Nullable<>))
                {
                    inner = valueType.GetProperty("Value")?.GetValue(value) ?? value;
                }

                PushTable(L, inner, nullableUnderlying, members);
                return;
            }

            if (value == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            LuaDll.lua_createtable(L, 0, members.Length);
            int tableIdx = LuaDll.lua_gettop(L);
            for (int i = 0; i < members.Length; i++)
            {
                LuaMarshalMemberBinding entry = members[i];
                object memberValue = GetMember(value, entry.Member);
                Type memberType = GetMemberType(entry.Member);
                TypedMarshal.PushObject(L, memberValue, memberType);
                LuaDll.lua_setfield(L, tableIdx, entry.ClrName);
            }
        }

        internal static object PopUnpacked(IntPtr L, int firstIdx, Type declaredType, LuaMarshalMemberBinding[] members)
        {
            object instance = Activator.CreateInstance(declaredType);
            for (int i = 0; i < members.Length; i++)
            {
                LuaMarshalMemberBinding entry = members[i];
                Type memberType = GetMemberType(entry.Member);
                object memberValue = TypedMarshal.PopObject(L, firstIdx + i, memberType);
                SetMember(instance, entry.Member, memberValue);
            }

            return instance;
        }

        internal static void PushUnpacked(IntPtr L, object value, Type declaredType, LuaMarshalMemberBinding[] members)
        {
            if (value == null)
            {
                LuaCallbackBoundary.Throw(
                    $"zlua: cannot push null as UnpackedValues ({declaredType.FullName})");
            }

            for (int i = 0; i < members.Length; i++)
            {
                LuaMarshalMemberBinding entry = members[i];
                object memberValue = GetMember(value, entry.Member);
                Type memberType = GetMemberType(entry.Member);
                TypedMarshal.PushObject(L, memberValue, memberType);
            }
        }

        internal static Type GetMemberType(MemberInfo member)
        {
            if (member is FieldInfo field)
            {
                return field.FieldType;
            }

            if (member is PropertyInfo property)
            {
                return property.PropertyType;
            }

            LuaCallbackBoundary.Throw("zlua internal error: composite member is not field/property");
            return null;
        }

        private static object GetMember(object instance, MemberInfo member)
        {
            if (member is FieldInfo field)
            {
                return field.GetValue(instance);
            }

            if (member is PropertyInfo property)
            {
                return property.GetValue(instance, null);
            }

            LuaCallbackBoundary.Throw("zlua internal error: composite member is not field/property");
            return null;
        }

        private static void SetMember(object instance, MemberInfo member, object value)
        {
            if (member is FieldInfo field)
            {
                field.SetValue(instance, value);
                return;
            }

            if (member is PropertyInfo property)
            {
                property.SetValue(instance, value, null);
                return;
            }

            LuaCallbackBoundary.Throw("zlua internal error: composite member is not field/property");
        }
    }
}
