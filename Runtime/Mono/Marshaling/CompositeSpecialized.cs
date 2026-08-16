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
using System.Reflection;
using ZLua.Utils;

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
            public readonly LuaMarshalType MarshalType;

            public CacheKey(Type declaredType, LuaMarshalType marshalType)
            {
                DeclaredType = declaredType;
                MarshalType = marshalType;
            }

            public bool Equals(CacheKey other) =>
                DeclaredType == other.DeclaredType && MarshalType == other.MarshalType;

            public override bool Equals(object obj) => obj is CacheKey other && Equals(other);

            public override int GetHashCode() =>
                ((DeclaredType != null ? DeclaredType.GetHashCode() : 0) * 397) ^ (int)MarshalType;
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

            if (Nullable.GetUnderlyingType(declaredType) != null)
            {
                return false;
            }

            if (!IsEligibleTypeLevelAllFields(declaredType, binding))
            {
                return false;
            }

            var key = new CacheKey(declaredType, binding.MarshalType);
            if (s_negative.ContainsKey(key))
            {
                return false;
            }

            if (s_cache.TryGetValue(key, out entry))
            {
                return true;
            }

            try
            {
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
                if (!(binding.Members[i].Member is FieldInfo field) || field.IsStatic)
                {
                    return false;
                }
            }

            return true;
        }

        private static Entry Build(Type declaredType, LuaMarshalBinding binding)
        {
            LuaMarshalMemberBinding[] members = binding.Members;
            LuaMarshalType marshalType = binding.MarshalType;
            int slots = binding.StackSlots;

            if (marshalType == LuaMarshalType.Table)
            {
                return new Entry
                {
                    Pop = (L, idx) => CompositeMarshal.PopTable(L, idx, declaredType, members),
                    Push = (L, value) =>
                    {
                        CompositeMarshal.PushTable(L, value, declaredType, members);
                        return 1;
                    },
                };
            }

            return new Entry
            {
                Pop = (L, idx) => CompositeMarshal.PopUnpacked(L, idx, declaredType, members),
                Push = (L, value) =>
                {
                    CompositeMarshal.PushUnpacked(L, value, declaredType, members);
                    return slots;
                },
            };
        }
    }
}
