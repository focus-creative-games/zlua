using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Lua table → C# struct default assembly (STRUCT_MARSHAL_SPEC §6.4).
    /// </summary>
    internal static class StructMarshaling
    {
        internal static bool CanComposeStructFromTable(IntPtr luaState, int luaIndex, Type structType)
        {
            if (!ValueTypeMarshaling.IsStructType(structType))
            {
                return false;
            }

            if (LuaDll.lua_type(luaState, luaIndex) != LuaDataType.Table)
            {
                return false;
            }

            return ValidateRequiredTableFields(luaState, luaIndex, structType, out _);
        }

        internal static bool TryComposeStructFromTable(IntPtr luaState, int luaIndex, Type structType, out object value)
        {
            value = null;
            if (!ValueTypeMarshaling.IsStructType(structType))
            {
                return false;
            }

            if (LuaDll.lua_type(luaState, luaIndex) != LuaDataType.Table)
            {
                return false;
            }

            if (!ValidateRequiredTableFields(luaState, luaIndex, structType, out string error))
            {
                throw new NotSupportedException(error);
            }

            object instance = Activator.CreateInstance(structType);
            FieldInfo[] fields = structType.GetFields(BindingFlags.Public | BindingFlags.Instance);
            int absIndex = LuaDll.lua_absindex(luaState, luaIndex);

            for (int i = 0; i < fields.Length; i++)
            {
                FieldInfo field = fields[i];
                if (field.IsStatic)
                {
                    continue;
                }

                if (!TryReadTableField(luaState, absIndex, field, out object fieldValue))
                {
                    continue;
                }

                field.SetValue(instance, fieldValue);
            }

            value = instance;
            return true;
        }

        private static bool ValidateRequiredTableFields(IntPtr luaState, int luaIndex, Type structType, out string error)
        {
            error = null;
            FieldInfo[] fields = structType.GetFields(BindingFlags.Public | BindingFlags.Instance);
            int absIndex = LuaDll.lua_absindex(luaState, luaIndex);

            for (int i = 0; i < fields.Length; i++)
            {
                FieldInfo field = fields[i];
                if (field.IsStatic)
                {
                    continue;
                }

                if (!TryGetTableFieldType(luaState, absIndex, field.Name, out LuaDataType fieldType))
                {
                    error = $"struct table missing required field {structType.Name}.{field.Name}";
                    return false;
                }

                if (fieldType == LuaDataType.Nil)
                {
                    LuaDll.lua_pop(luaState, 1);
                    if (IsOptionalField(field))
                    {
                        continue;
                    }

                    error = $"struct table missing required field {structType.Name}.{field.Name}";
                    return false;
                }

                if (!CanConvertFieldValue(luaState, -1, field.FieldType))
                {
                    error = $"struct table field {structType.Name}.{field.Name} has unsupported value";
                    return false;
                }

                LuaDll.lua_pop(luaState, 1);
            }

            return true;
        }

        private static bool TryReadTableField(IntPtr luaState, int tableIndex, FieldInfo field, out object fieldValue)
        {
            fieldValue = null;
            if (!TryGetTableFieldType(luaState, tableIndex, field.Name, out LuaDataType fieldType))
            {
                return false;
            }

            if (fieldType == LuaDataType.Nil)
            {
                LuaDll.lua_pop(luaState, 1);
                return false;
            }

            try
            {
                fieldValue = TypeMethodRegistration.ReadArgumentValue(luaState, -1, field.FieldType);
            }
            finally
            {
                LuaDll.lua_pop(luaState, 1);
            }

            return true;
        }

        private static bool TryGetTableFieldType(IntPtr luaState, int tableIndex, string fieldName, out LuaDataType fieldType)
        {
            fieldType = LuaDataType.Nil;
            LuaDll.lua_pushstring(luaState, fieldName);
            fieldType = LuaDll.lua_rawget(luaState, tableIndex);
            return true;
        }

        private static bool CanConvertFieldValue(IntPtr luaState, int luaIndex, Type fieldType)
        {
            return TypeMethodRegistration.CanConvertArgumentValue(luaState, luaIndex, fieldType);
        }

        private static bool IsOptionalField(FieldInfo field)
        {
            object[] attributes = field.GetCustomAttributes(typeof(LuaMarshalAsAttribute), inherit: false);
            for (int i = 0; i < attributes.Length; i++)
            {
                if (attributes[i] is LuaMarshalAsAttribute marshalAs
                    && (marshalAs.Flags & LuaMarshalFlags.OptionalField) != 0)
                {
                    return true;
                }
            }

            return false;
        }
    }
}
