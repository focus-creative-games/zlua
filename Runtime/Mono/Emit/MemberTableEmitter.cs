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
                        try
                        {
                            FieldEmitter.Write(
                                L,
                                tables.FieldGetterTableRef,
                                tables.FieldSetterTableRef,
                                info.Field,
                                isStatic,
                                isByVal);
                        }
                        catch (EmitException)
                        {
                            if (IsDeclaredOn(info.Field?.DeclaringType, binding.Type))
                            {
                                throw;
                            }
                        }

                        break;

                    case MetaKind.Property:
                        try
                        {
                            PropertyEmitter.Write(
                                L,
                                tables.FieldGetterTableRef,
                                tables.FieldSetterTableRef,
                                info.Property,
                                isStatic,
                                isByVal);
                        }
                        catch (EmitException)
                        {
                            if (IsDeclaredOn(info.Property?.DeclaringType, binding.Type))
                            {
                                throw;
                            }
                        }

                        break;

                    case MetaKind.Method:
                        try
                        {
                            MethodEmitter.Write(L, tables.MethodTableRef, info, binding.Type, isStatic, isByVal);
                        }
                        catch (EmitException)
                        {
                            Type declared = info.Method?.DeclaringType
                                            ?? (info.MethodOverloads != null && info.MethodOverloads.Count > 0
                                                ? info.MethodOverloads[0].DeclaringType
                                                : null);
                            if (IsDeclaredOn(declared, binding.Type))
                            {
                                throw;
                            }
                        }

                        break;
                }
            }
        }

        private static bool IsDeclaredOn(Type memberDeclaringType, Type bindingType)
        {
            return memberDeclaringType != null && memberDeclaringType == bindingType;
        }
    }
}
