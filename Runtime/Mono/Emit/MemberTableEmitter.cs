using System;
using System.Collections.Generic;
using ZLua.Mt;

namespace ZLua.Emit
{
    /// <summary>
    /// Fills registry three-tables and SMT.__call after empty tables are created.
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
