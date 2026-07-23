using System;
using System.Reflection;
using ZLua.Utils;

namespace ZLua.Emit
{
    internal static class FieldEmitter
    {
        internal static void Write(IntPtr L, int getterTableRef, int setterTableRef, FieldInfo field, bool isStatic, bool isByVal)
        {
            if (field == null)
            {
                return;
            }

            if (!BridgeMarshaling.IsSupportedType(field.FieldType))
            {
                throw EmitException.ForMember(field.DeclaringType, field.Name, "unsupported field type");
            }

            ClosurePin.WriteToTable(L, getterTableRef, field.Name, CompileGetter(field, isStatic, isByVal));

            if (!field.IsInitOnly && !field.IsLiteral)
            {
                ClosurePin.WriteToTable(L, setterTableRef, field.Name, CompileSetter(field, isStatic, isByVal));
            }
        }

        private static LuaCSFunction CompileGetter(FieldInfo field, bool isStatic, bool isByVal)
        {
            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;

            return L =>
            {
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        object target = null;
                        if (!isStatic)
                        {
                            target = BridgeMarshaling.PopTarget(L, 1, declaringType, isByVal);
                        }

                        object value = field.GetValue(target);
                        return BridgeMarshaling.PushReturn(L, fieldType, value);
                    }
                    catch (Exception ex)
                    {
                        return LuaCallbackBoundary.ToLuaError(L, ex);
                    }
                }
                finally
                {
                    StructOpaqueScope.LeaveLuaToCSharp();
                }
            };
        }

        private static LuaCSFunction CompileSetter(FieldInfo field, bool isStatic, bool isByVal)
        {
            Type declaringType = field.DeclaringType;
            Type fieldType = field.FieldType;

            return L =>
            {
                StructOpaqueScope.EnterLuaToCSharp();
                try
                {
                    try
                    {
                        if (isStatic)
                        {
                            object value = BridgeMarshaling.PopArg(L, 2, fieldType);
                            field.SetValue(null, value);
                            return 0;
                        }

                        object valueInst = BridgeMarshaling.PopArg(L, 2, fieldType);
                        BridgeMarshaling.SetInstanceField(L, 1, field, valueInst, isByVal);
                        return 0;
                    }
                    catch (Exception ex)
                    {
                        return LuaCallbackBoundary.ToLuaError(L, ex);
                    }
                }
                finally
                {
                    StructOpaqueScope.LeaveLuaToCSharp();
                }
            };
        }
    }
}
