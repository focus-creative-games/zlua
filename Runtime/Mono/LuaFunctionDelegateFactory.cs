using System;
using System.Reflection;

namespace NovaLua
{
    public static class LuaFunctionDelegateFactory
    {
        public static Delegate Create(MethodInfo method)
        {
            if (method.GetCustomAttribute<LuaInvokeAttribute>() == null)
            {
                throw new ArgumentException("Method must be marked with [LuaInvoke].", nameof(method));
            }

            throw new NotSupportedException("v0.0.1 does not provide reflection delegate generation yet.");
        }
    }
}
