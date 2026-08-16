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
using ZLua.Emit;

namespace ZLua.Mt
{
    internal static class TypeRegistryReference
    {
        internal static void CreateTypeTable(IntPtr L, Type type)
        {
            TypeBinding binding = MetaBinding.EnsureBinding(type);

            LuaDll.lua_createtable(L, 0, 8);
            int typeTableIndex = LuaDll.lua_gettop(L);

            TypeRegistryCommon.WriteCommonTypeFields(L, type, typeTableIndex);
            TypeRegistryCommon.RegisterStaticLiteralFields(L, type, typeTableIndex);
            TypeRegistryCommon.AttachReferenceInstanceMetatable(L, type, typeTableIndex, binding);
            TypeRegistryCommon.AttachStaticTypeMetatable(
                L,
                type,
                typeTableIndex,
                binding,
                enableConstructorCall: HasPublicInstanceCtor(type),
                enableStructDefault: false);
            MemberTableEmitter.Fill(L, binding, typeTableIndex);
        }

        internal static bool HasPublicInstanceCtor(Type type)
        {
            if (type == null || type.IsInterface || type.IsAbstract)
            {
                return false;
            }

            ConstructorInfo[] ctors = type.GetConstructors(BindingFlags.Public | BindingFlags.Instance);
            return ctors.Length > 0;
        }
    }
}
