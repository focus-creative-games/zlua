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

namespace ZLua
{
    public enum LuaMarshalType
    {
        Default,
        UserData,
        Bytes,
        OpaqueValue,
        UnpackedValues,
        Table,
    }

    [AttributeUsage(
        AttributeTargets.Parameter
        | AttributeTargets.ReturnValue
        | AttributeTargets.Field
        | AttributeTargets.Property
        | AttributeTargets.Class
        | AttributeTargets.Struct)]
    public sealed class LuaMarshalAsAttribute : Attribute
    {
        public LuaMarshalType LuaMarshalType { get; }

        /// <summary>
        /// Required for <see cref="LuaMarshalType.Table"/> / <see cref="LuaMarshalType.UnpackedValues"/>.
        /// Elements are CLR field or property names on the underlying struct (Nullable unwraps to T).
        /// Trailing '?' marks optional Table keys (Lua→C#). UnpackedValues does not support '?'.
        /// </summary>
        public string[] Members { get; set; }

        public LuaMarshalAsAttribute(LuaMarshalType luaMarshalType = LuaMarshalType.Default)
        {
            LuaMarshalType = luaMarshalType;
        }
    }
}
