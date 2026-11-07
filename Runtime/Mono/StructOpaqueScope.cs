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
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace ZLua
{
    /// <summary>
    /// Sync-call struct opaque handles (MARSHAL_SPEC §4, OpaqueValue).
    /// Handles live until their synchronous C#→Lua frame exits, including nested callbacks.
    /// </summary>
    internal static class StructOpaqueScope
    {
        private static readonly object Sync = new object();
        private static uint _generation;
        private static readonly List<(GCHandle handle, Type type, uint generation)> Entries =
            new List<(GCHandle, Type, uint)>();

        /// <summary>Save the first slot owned by this C#→Lua frame.</summary>
        internal static int EnterCSharpToLua()
        {
            lock (Sync)
            {
                return Entries.Count;
            }
        }

        /// <summary>Free only this frame's handles; outer frames remain valid.</summary>
        internal static void LeaveCSharpToLua(int firstSlot)
        {
            lock (Sync)
            {
                for (int i = Entries.Count - 1; i >= firstSlot; i--)
                {
                    Entries[i].handle.Free();
                }
                if (firstSlot < Entries.Count)
                    Entries.RemoveRange(firstSlot, Entries.Count - firstSlot);
            }
        }

        /// <summary>
        /// Drop opaque handles. Call on Reset / <c>lua_close</c>.
        /// </summary>
        internal static void Shutdown()
        {
            LeaveCSharpToLua(0);
        }

        internal static IntPtr RegisterStruct(object boxedStruct, Type structType)
        {
            if (boxedStruct == null)
            {
                return IntPtr.Zero;
            }

            lock (Sync)
            {
                // Never revive a stale handle when a released slot is reused, even after Reset.
                if (_generation == uint.MaxValue)
                    throw new InvalidOperationException("Opaque handle generation exhausted.");
                uint generation = ++_generation;
                int index = Entries.Count;
                GCHandle handle = GCHandle.Alloc(boxedStruct);
                try { Entries.Add((handle, structType, generation)); }
                catch { handle.Free(); throw; }
                return EncodeHandle(generation, index);
            }
        }

        internal static bool TryResolve(IntPtr handleId, Type expectedType, out object value)
        {
            value = null;
            if (handleId == IntPtr.Zero || expectedType == null)
            {
                return false;
            }

            if (!TryResolveEntry(handleId, out object entryValue, out Type entryType))
            {
                return false;
            }

            if (entryType != expectedType || entryValue == null || entryValue.GetType() != expectedType)
            {
                return false;
            }

            value = entryValue;
            return true;
        }

        internal static bool TryResolveEntry(IntPtr handleId, out object value, out Type valueType)
        {
            value = null;
            valueType = null;
            if (handleId == IntPtr.Zero)
            {
                return false;
            }

            if (!TryDecodeHandle(handleId, out uint generation, out int index))
            {
                return false;
            }

            lock (Sync)
            {
                if (index < 0 || index >= Entries.Count || generation != Entries[index].generation)
                {
                    return false;
                }

                var entry = Entries[index];
                if (!entry.handle.IsAllocated)
                {
                    return false;
                }

                value = entry.handle.Target;
                valueType = entry.type;
                return value != null && valueType != null;
            }
        }

        internal static bool TryUpdate(IntPtr handleId, object newValue)
        {
            if (newValue == null || handleId == IntPtr.Zero)
            {
                return false;
            }

            if (!TryDecodeHandle(handleId, out uint generation, out int index))
            {
                return false;
            }

            lock (Sync)
            {
                if (index < 0 || index >= Entries.Count || generation != Entries[index].generation)
                {
                    return false;
                }

                var entry = Entries[index];
                if (!entry.handle.IsAllocated)
                {
                    return false;
                }

                if (entry.type == null || newValue.GetType() != entry.type)
                {
                    return false;
                }

                entry.handle.Target = newValue;
                return true;
            }
        }

        private static IntPtr EncodeHandle(uint generation, int index)
        {
            long encoded = ((long)generation << 32) | (uint)(index + 1);
            return new IntPtr(encoded);
        }

        private static bool TryDecodeHandle(IntPtr handleId, out uint generation, out int index)
        {
            generation = 0;
            index = -1;
            long encoded = handleId.ToInt64();
            if (encoded == 0)
            {
                return false;
            }

            generation = (uint)(encoded >> 32);
            uint slot = (uint)(encoded & 0xFFFFFFFF);
            if (slot == 0)
            {
                return false;
            }

            index = (int)(slot - 1);
            return true;
        }
    }
}
