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
    /// Handles from the current invoke remain valid until the next Lua→C# entry at depth 0.
    /// </summary>
    internal static class StructOpaqueScope
    {
        private static readonly object Sync = new object();
        private static uint _generation = 1;
        private static int _luaToCSharpDepth;
        private static readonly List<(GCHandle handle, Type type)> Entries = new List<(GCHandle, Type)>();

        /// <summary>True while executing a Lua→C# bridge callback (MethodEmitter Wrap, etc.).</summary>
        internal static bool IsInsideLuaToCSharp => _luaToCSharpDepth > 0;

        /// <summary>Lua→C# method/constructor callback entry.</summary>
        internal static void EnterLuaToCSharp()
        {
            if (_luaToCSharpDepth++ == 0)
            {
                ResetScope();
            }
        }

        /// <summary>Lua→C# method/constructor callback exit.</summary>
        internal static void LeaveLuaToCSharp()
        {
            if (_luaToCSharpDepth > 0)
            {
                _luaToCSharpDepth--;
            }
        }

        /// <summary>Standalone C#→Lua (GetFunction / top-level pcall), only when not nested in Lua→C#.</summary>
        internal static void EnterStandaloneCSharpToLua()
        {
            if (_luaToCSharpDepth == 0)
            {
                ResetScope();
            }
        }

        internal static IntPtr RegisterStruct(object boxedStruct, Type structType)
        {
            if (boxedStruct == null)
            {
                return IntPtr.Zero;
            }

            GCHandle handle = GCHandle.Alloc(boxedStruct);
            lock (Sync)
            {
                int index = Entries.Count;
                Entries.Add((handle, structType));
                return EncodeHandle(_generation, index);
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
                if (generation != _generation || index < 0 || index >= Entries.Count)
                {
                    return false;
                }

                (GCHandle handle, Type type) entry = Entries[index];
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
                if (generation != _generation || index < 0 || index >= Entries.Count)
                {
                    return false;
                }

                (GCHandle handle, Type type) entry = Entries[index];
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

        private static void ResetScope()
        {
            lock (Sync)
            {
                for (int i = 0; i < Entries.Count; i++)
                {
                    if (Entries[i].handle.IsAllocated)
                    {
                        Entries[i].handle.Free();
                    }
                }

                Entries.Clear();
                _generation++;
                if (_generation == 0)
                {
                    _generation = 1;
                }
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
