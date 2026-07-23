using System;
using ZLua.Emit;
using ZLua.Marshaling;

namespace ZLua.Mt
{
    internal static class TypeRegistryValueType
    {
        internal static void CreateTypeTable(IntPtr L, Type type)
        {
            // Non-enum, non-nullable struct (and primitive boxed path uses ByObj only).
            TypeBinding binding = MetaBinding.EnsureBinding(type);

            LuaDll.lua_createtable(L, 0, 8);
            int typeTableIndex = LuaDll.lua_gettop(L);

            TypeRegistryCommon.WriteCommonTypeFields(L, type, typeTableIndex);
            TypeRegistryCommon.RegisterStaticLiteralFields(L, type, typeTableIndex);

            if (!type.IsPrimitive)
            {
                LuaDll.lua_pushboolean(L, 1);
                LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.Struct);
            }

            bool isStruct = StructMarshal.IsStructType(type);
            if (isStruct)
            {
                TypeRegistryCommon.AttachByValInstanceMetatable(L, type, typeTableIndex, binding);
            }

            TypeRegistryCommon.AttachByObjInstanceMetatable(L, type, typeTableIndex, binding);
            TypeRegistryCommon.AttachStaticTypeMetatable(
                L,
                type,
                typeTableIndex,
                binding,
                enableConstructorCall: isStruct,
                enableStructDefault: isStruct);
            MemberTableEmitter.Fill(L, binding, typeTableIndex);
        }

        internal static void CreateEnumTypeTable(IntPtr L, Type type)
        {
            TypeBinding binding = MetaBinding.EnsureBinding(type);

            LuaDll.lua_createtable(L, 0, 8);
            int typeTableIndex = LuaDll.lua_gettop(L);

            TypeRegistryCommon.WriteCommonTypeFields(L, type, typeTableIndex);
            LuaDll.lua_pushboolean(L, 1);
            LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.Enum);
            TypeRegistryCommon.RegisterStaticLiteralFields(L, type, typeTableIndex);
            TypeRegistryCommon.AttachByObjInstanceMetatable(L, type, typeTableIndex, binding);
            TypeRegistryCommon.AttachStaticTypeMetatable(
                L,
                type,
                typeTableIndex,
                binding,
                enableConstructorCall: false,
                enableStructDefault: false);
            MemberTableEmitter.Fill(L, binding, typeTableIndex);
        }

        internal static void CreateNullableTypeTable(IntPtr L, Type type)
        {
            TypeBinding binding = MetaBinding.EnsureBinding(type);

            LuaDll.lua_createtable(L, 0, 8);
            int typeTableIndex = LuaDll.lua_gettop(L);

            TypeRegistryCommon.WriteCommonTypeFields(L, type, typeTableIndex);
            LuaDll.lua_pushboolean(L, 1);
            LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.Nullable);

            TypeRegistryCommon.AttachStaticTypeMetatable(
                L,
                type,
                typeTableIndex,
                binding,
                enableConstructorCall: false,
                enableStructDefault: false);
            MemberTableEmitter.Fill(L, binding, typeTableIndex);
        }
    }
}
