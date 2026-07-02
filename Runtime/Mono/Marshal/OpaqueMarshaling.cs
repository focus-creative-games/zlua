using System;
using ZLua.Mt;
using ZLua;

namespace ZLua.Marshal
{
    internal static class OpaqueMarshaling
    {
        internal static int ToUserData(IntPtr luaState, int opaqueIndex)
        {
            if (LuaDll.lua_type(luaState, opaqueIndex) != LuaDataType.LightUserData)
            {
                return LuaDllExtension.error(luaState, "zlua.to_user_data: opaque must be lightuserdata");
            }

            IntPtr handleId = LuaDll.lua_touserdata(luaState, opaqueIndex);
            if (!StructOpaqueScope.TryResolveEntry(handleId, out object value, out Type valueType))
            {
                return LuaDllExtension.error(luaState, "zlua.to_user_data: opaque handle is invalid or expired");
            }

            if (StructMarshaling.IsStructType(valueType))
            {
                object copy = StructMarshaling.CopyBoxedStruct(value, valueType);
                return TypeRegistry.PushConstructorInstance(luaState, copy, valueType);
            }

            if (valueType.IsClass || valueType.IsInterface)
            {
                return TypeRegistry.PushConstructorInstance(luaState, value, valueType);
            }

            return LuaDllExtension.error(luaState, $"zlua.to_user_data: unsupported opaque type {valueType.FullName}");
        }
    }
}
