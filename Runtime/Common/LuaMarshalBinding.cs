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

using System.Reflection;

namespace ZLua
{
    public enum LuaMarshalDirection
    {
        LuaToCSharp,
        CSharpToLua,
    }

    /// <summary>
    /// Resolved marshal metadata for one parameter or return value (MARSHAL_SPEC §6.4).
    /// </summary>
    public sealed class LuaMarshalBinding
    {
        public LuaMarshalType MarshalType { get; }

        public LuaMarshalMemberBinding[] Members { get; }

        public LuaMarshalBinding(LuaMarshalType marshalType, LuaMarshalMemberBinding[] members = null)
        {
            MarshalType = marshalType;
            Members = members ?? System.Array.Empty<LuaMarshalMemberBinding>();
        }

        /// <summary>
        /// Lua stack slots occupied by this parameter/return (UnpackedValues = Members.Length; else 1).
        /// </summary>
        public int StackSlots =>
            MarshalType == LuaMarshalType.UnpackedValues ? Members.Length : 1;

        public bool IsComposite =>
            MarshalType == LuaMarshalType.Table || MarshalType == LuaMarshalType.UnpackedValues;

        public static LuaMarshalBinding Default { get; } = new LuaMarshalBinding(LuaMarshalType.Default);
    }

    public sealed class LuaMarshalMemberBinding
    {
        public string ClrName { get; }

        public bool IsOptional { get; }

        public MemberInfo Member { get; }

        public LuaMarshalMemberBinding(string clrName, bool isOptional, MemberInfo member)
        {
            ClrName = clrName;
            IsOptional = isOptional;
            Member = member;
        }
    }
}
