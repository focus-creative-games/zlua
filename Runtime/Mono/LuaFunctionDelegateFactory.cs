using System;
using System.Reflection;

namespace ZLua
{
    public static class LuaFunctionDelegateFactory
    {
        public static Delegate Create(LuaEnv env, Type delegateType, int funcRef)
        {
            return LuaDelegateBinder.Create(env, delegateType, funcRef);
        }

        public static Delegate Create(MethodInfo method)
        {
            throw new NotSupportedException("Use LuaDelegateBinder.Create with delegate type and funcRef.");
        }
    }
}
