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
using ZLua.Mt;

namespace ZLua.Emit
{
    /// <summary>
    /// Fills registry three-tables and SMT.__call after empty tables are created.
    /// Method/Field/Property entries are lazy stubs (Expression.Compile on first use);
    /// <see cref="ConstructorEmitter.BindCall"/> remains eager.
    /// </summary>
    internal static class MemberTableEmitter
    {
        internal static void Fill(IntPtr L, TypeBinding binding, int typeTableIndex)
        {
            if (binding == null)
            {
                return;
            }

            if (binding.StaticTables != null && binding.StaticTables.IsValid)
            {
                FillMap(L, binding, binding.StaticTables, binding.StaticMap, isStatic: true, isByVal: false);
            }

            if (binding.ByObjInstanceTables != null && binding.ByObjInstanceTables.IsValid)
            {
                FillMap(L, binding, binding.ByObjInstanceTables, binding.ByObjInstanceMap, isStatic: false, isByVal: false);
            }

            if (binding.ByValInstanceTables != null && binding.ByValInstanceTables.IsValid)
            {
                FillMap(L, binding, binding.ByValInstanceTables, binding.ByValInstanceMap, isStatic: false, isByVal: true);
            }

            ConstructorEmitter.BindCall(L, typeTableIndex, binding);
        }

        private static void FillMap(
            IntPtr L,
            TypeBinding binding,
            MemberTableSet tables,
            Dictionary<string, MetaInfo> map,
            bool isStatic,
            bool isByVal)
        {
            foreach (KeyValuePair<string, MetaInfo> kv in map)
            {
                MetaInfo info = kv.Value;
                switch (info.Kind)
                {
                    case MetaKind.Field:
                        FieldEmitter.Write(
                            L,
                            tables.FieldGetterTableRef,
                            tables.FieldSetterTableRef,
                            info.Field,
                            isStatic,
                            isByVal);
                        break;

                    case MetaKind.Property:
                        PropertyEmitter.Write(
                            L,
                            tables.FieldGetterTableRef,
                            tables.FieldSetterTableRef,
                            info.Property,
                            isStatic,
                            isByVal);
                        break;

                    case MetaKind.Method:
                        MethodEmitter.Write(L, tables.MethodTableRef, info, binding.Type, isStatic, isByVal);
                        break;
                }
            }
        }
    }
}
