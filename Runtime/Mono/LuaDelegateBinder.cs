using System;
using System.Reflection;

namespace ZLua
{
    internal static class LuaDelegateBinder
    {
        internal static Delegate Create(LuaEnv env, Type delegateType, int funcRef)
        {
            if (env == null)
            {
                throw new ArgumentNullException(nameof(env));
            }

            if (delegateType == null)
            {
                throw new ArgumentNullException(nameof(delegateType));
            }

            if (!typeof(Delegate).IsAssignableFrom(delegateType))
            {
                throw new ArgumentException($"Type '{delegateType.FullName}' is not a delegate type.", nameof(delegateType));
            }

            MethodInfo invokeMethod = delegateType.GetMethod("Invoke");
            if (invokeMethod == null)
            {
                throw new InvalidOperationException($"Delegate type '{delegateType.FullName}' has no Invoke method.");
            }

            LuaMethod target = new LuaMethod(env, funcRef);
            return DynamicBridgeFactory.CreateDelegate(delegateType, target);
        }

        internal static int CreateFunctionRef(IntPtr luaState, int index)
        {
            LuaDll.lua_pushvalue(luaState, index);
            int funcRef = LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
            if (funcRef < 0)
            {
                throw new InvalidOperationException("Failed to create lua function reference.");
            }

            return funcRef;
        }
    }
}
