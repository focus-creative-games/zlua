using System;
using ZLua;
using ZLua.DelegateImpl;
using ZLua.Lvm;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class DelegateMarshal
    {
        internal static void Push(IntPtr L, Delegate del, Type viewType = null)
        {
            if (del == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            if (del.Target is LuaMethod luaMethod)
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, luaMethod.RefIndex);
                return;
            }

            ObjectMarshal.Push(L, del, viewType ?? del.GetType());
        }

        internal static Delegate Pop(IntPtr L, int index, Type delegateType)
        {
            if (delegateType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: delegateType is null");
            }

            if (!typeof(Delegate).IsAssignableFrom(delegateType))
            {
                LuaCallbackBoundary.Throw("zlua: argument mismatch: expected delegate");
            }

            LuaDataType valueType = LuaDll.lua_type(L, index);
            switch (valueType)
            {
                case LuaDataType.Function:
                {
                    LuaEnv env = LuaEnv.Active;
                    if (env == null)
                    {
                        LuaCallbackBoundary.Throw("zlua internal error: no active LuaEnv");
                    }

                    int funcRef = LuaDelegateBinder.CreateFunctionRef(L, index);
                    return LuaDelegateBinder.Create(env, delegateType, funcRef);
                }

                case LuaDataType.UserData:
                {
                    object obj = ObjectRegistry.Pop(L, index);
                    if (obj == null || !delegateType.IsInstanceOfType(obj))
                    {
                        LuaCallbackBoundary.Throw("zlua: argument mismatch: expected delegate");
                    }

                    return (Delegate)obj;
                }

                case LuaDataType.Nil:
                    return null;

                default:
                    LuaCallbackBoundary.Throw("zlua: argument mismatch: expected delegate");
                    return null;
            }
        }
    }
}
