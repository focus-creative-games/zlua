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
