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
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.CompilerServices;
using ZLua.Utils;
using ZLua.Emit;

namespace ZLua.Marshaling
{
    /// <summary>
    /// Type-level Table/UnpackedValues with all-field Members → cached specialized pop/push.
    /// </summary>
    internal static class CompositeSpecialized
    {
        private delegate object PopFn(IntPtr L, int valueIdx);

        private delegate int PushFn(IntPtr L, object value);

        private readonly struct CacheKey : IEquatable<CacheKey>
        {
            public readonly Type DeclaredType;
            public readonly LuaMarshalBinding Binding;

            public CacheKey(Type declaredType, LuaMarshalBinding binding)
            {
                DeclaredType = declaredType;
                Binding = binding;
            }

            public bool Equals(CacheKey other) =>
                DeclaredType == other.DeclaredType && ReferenceEquals(Binding, other.Binding);

            public override bool Equals(object obj) => obj is CacheKey other && Equals(other);

            public override int GetHashCode() =>
                ((DeclaredType != null ? DeclaredType.GetHashCode() : 0) * 397) ^
                (Binding != null ? RuntimeHelpers.GetHashCode(Binding) : 0);
        }

        private sealed class Entry
        {
            public PopFn Pop;
            public PushFn Push;
        }

        private static readonly ConcurrentDictionary<CacheKey, Entry> s_cache =
            new ConcurrentDictionary<CacheKey, Entry>();

        private static readonly ConcurrentDictionary<CacheKey, byte> s_negative =
            new ConcurrentDictionary<CacheKey, byte>();

        internal static void Shutdown()
        {
            s_cache.Clear();
            s_negative.Clear();
        }

        internal static bool TryPop(
            IntPtr L,
            int valueIdx,
            Type declaredType,
            LuaMarshalBinding binding,
            out object value)
        {
            value = null;
            if (!TryGetEntry(declaredType, binding, out Entry entry))
            {
                return false;
            }

            value = entry.Pop(L, valueIdx);
            return true;
        }

        internal static bool TryPush(
            IntPtr L,
            object value,
            Type declaredType,
            LuaMarshalBinding binding,
            out int stackSlots)
        {
            stackSlots = 0;
            if (!TryGetEntry(declaredType, binding, out Entry entry))
            {
                return false;
            }

            stackSlots = entry.Push(L, value);
            return true;
        }

        private static bool TryGetEntry(Type declaredType, LuaMarshalBinding binding, out Entry entry)
        {
            entry = null;
            if (declaredType == null || binding == null || !binding.IsComposite)
            {
                return false;
            }

            // Build captures this binding's Members, their order/optionality and slot
            // count. Reuse only the same resolved binding, without per-call member scans.
            var key = new CacheKey(declaredType, binding);
            if (s_cache.TryGetValue(key, out entry))
            {
                return true;
            }
            if (s_negative.ContainsKey(key))
            {
                return false;
            }

            try
            {
                if (!IsEligibleTypeLevelAllFields(declaredType, binding))
                {
                    s_negative[key] = 0;
                    return false;
                }
                entry = Build(declaredType, binding);
                s_cache[key] = entry;
                return true;
            }
            catch
            {
                s_negative[key] = 0;
                return false;
            }
        }

        private static bool IsEligibleTypeLevelAllFields(Type declaredType, LuaMarshalBinding binding)
        {
            if (!declaredType.IsValueType || Nullable.GetUnderlyingType(declaredType) != null)
            {
                return false;
            }

            LuaMarshalAsAttribute typeAttr = declaredType.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (typeAttr == null || typeAttr.LuaMarshalType != binding.MarshalType)
            {
                return false;
            }

            if (binding.Members == null || binding.Members.Length == 0)
            {
                return false;
            }

            for (int i = 0; i < binding.Members.Length; i++)
            {
                if (!(binding.Members[i].Member is FieldInfo field) || field.IsStatic || field.IsInitOnly)
                {
                    return false;
                }
            }

            return true;
        }

        private const BindingFlags StaticAny = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;
        private static readonly MethodInfo s_absIndex = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_absindex));
        private static readonly MethodInfo s_getField = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_getfield));
        private static readonly MethodInfo s_setField = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_setfield));
        private static readonly MethodInfo s_getTop = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_gettop));
        private static readonly MethodInfo s_createTable = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_createtable));
        private static readonly MethodInfo s_pushNil = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_pushnil));
        private static readonly MethodInfo s_pop = typeof(LuaDll).GetMethod(nameof(LuaDll.lua_pop));
        private static readonly MethodInfo s_requireTable = typeof(CompositeSpecialized).GetMethod(nameof(RequireTable), StaticAny);
        private static readonly MethodInfo s_missingKey = typeof(CompositeSpecialized).GetMethod(nameof(MissingKey), StaticAny);
        private static readonly MethodInfo s_nullUnpacked = typeof(CompositeSpecialized).GetMethod(nameof(NullUnpacked), StaticAny);

        private static Entry Build(Type declaredType, LuaMarshalBinding binding)
        {
            return new Entry
            {
                Pop = BuildPop(declaredType, binding),
                Push = BuildPush(declaredType, binding),
            };
        }

        private static PopFn BuildPop(Type declaredType, LuaMarshalBinding binding)
        {
            var L = Expression.Parameter(typeof(IntPtr), "L");
            var index = Expression.Parameter(typeof(int), "index");
            var firstIndex = Expression.Variable(typeof(int), "firstIndex");
            var value = Expression.Variable(declaredType, "value");
            var expressions = new List<Expression>
            {
                Expression.Assign(firstIndex, Expression.Call(s_absIndex, L, index)),
            };
            bool table = binding.MarshalType == LuaMarshalType.Table;
            if (table)
            {
                expressions.Add(Expression.Call(s_requireTable, L, firstIndex, Expression.Constant(declaredType)));
            }
            // A typed local avoids Activator and per-field boxing. The object-based
            // caller still requires one box when the completed struct is returned.
            expressions.Add(Expression.Assign(value, Expression.New(declaredType)));
            for (int i = 0; i < binding.Members.Length; i++)
            {
                LuaMarshalMemberBinding member = binding.Members[i];
                var field = (FieldInfo)member.Member;
                Expression slot = table ? Expression.Constant(-1) : Expression.Add(firstIndex, Expression.Constant(i));
                // Composite string fields also accept ByObj string userdata,
                // unlike the primitive-only string Pop used by typed methods.
                Expression popped = field.FieldType == typeof(string)
                    ? Expression.Convert(Expression.Call(EmitMethods.PopArg, L, slot, Expression.Constant(typeof(string))), typeof(string))
                    : TypedBridgeEmit.PopArg(L, slot, field.FieldType);
                Expression assign = Expression.Assign(Expression.Field(value, field), popped);
                if (!table)
                {
                    expressions.Add(assign);
                    continue;
                }

                Expression missing = member.IsOptional
                    ? Expression.Empty()
                    : Expression.Call(s_missingKey, Expression.Constant(declaredType), Expression.Constant(member.ClrName));
                Expression read = Expression.IfThenElse(
                    Expression.Equal(
                        Expression.Call(s_getField, L, firstIndex, Expression.Constant(member.ClrName)),
                        Expression.Constant(LuaDataType.Nil)),
                    missing,
                    Expression.Block(assign, Expression.Empty()));
                // getfield leaves one value even when conversion / required-key
                // validation fails. Match the general path's stack cleanup.
                expressions.Add(Expression.TryFinally(read, Expression.Call(s_pop, L, Expression.Constant(1))));
            }
            expressions.Add(Expression.Convert(value, typeof(object)));
            return Expression.Lambda<PopFn>(Expression.Block(new[] { firstIndex, value }, expressions), L, index).Compile();
        }

        private static PushFn BuildPush(Type declaredType, LuaMarshalBinding binding)
        {
            var L = Expression.Parameter(typeof(IntPtr), "L");
            var boxed = Expression.Parameter(typeof(object), "boxed");
            var value = Expression.Variable(declaredType, "value");
            var tableIndex = Expression.Variable(typeof(int), "tableIndex");
            var expressions = new List<Expression>
            {
                Expression.Assign(value, Expression.Convert(boxed, declaredType)),
            };
            bool table = binding.MarshalType == LuaMarshalType.Table;
            if (table)
            {
                expressions.Add(Expression.Call(s_createTable, L, Expression.Constant(0), Expression.Constant(binding.Members.Length)));
                expressions.Add(Expression.Assign(tableIndex, Expression.Call(s_getTop, L)));
            }
            foreach (LuaMarshalMemberBinding member in binding.Members)
            {
                var field = (FieldInfo)member.Member;
                expressions.Add(TypedBridgeEmit.PushReturn(L, field.FieldType, Expression.Field(value, field)));
                if (table)
                {
                    expressions.Add(Expression.Call(s_setField, L, tableIndex, Expression.Constant(member.ClrName)));
                }
            }
            expressions.Add(Expression.Constant(binding.StackSlots));
            Expression nullCase = table
                ? Expression.Block(Expression.Call(s_pushNil, L), Expression.Constant(1))
                : Expression.Block(Expression.Call(s_nullUnpacked, Expression.Constant(declaredType)), Expression.Constant(binding.StackSlots));
            Expression body = Expression.Condition(
                Expression.Equal(boxed, Expression.Constant(null)),
                nullCase,
                Expression.Block(new[] { value, tableIndex }, expressions));
            return Expression.Lambda<PushFn>(body, L, boxed).Compile();
        }

        private static void RequireTable(IntPtr L, int index, Type declaredType)
        {
            if (!LuaDll.lua_istable(L, index))
            {
                LuaCallbackBoundary.Throw(
                    $"zlua: expected table for LuaMarshalType.Table ({declaredType.FullName}), got {LuaDll.lua_type(L, index)}");
            }
        }

        private static void MissingKey(Type declaredType, string name)
        {
            LuaCallbackBoundary.Throw($"zlua: missing table key '{name}' for {declaredType.FullName}");
        }

        private static void NullUnpacked(Type declaredType)
        {
            LuaCallbackBoundary.Throw($"zlua: cannot push null as UnpackedValues ({declaredType.FullName})");
        }
    }
}
