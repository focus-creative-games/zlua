using System;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace NextLua
{
    public static class LuaInvokeFunctions
    {
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Run(params object[] args)
        {
            (string moduleName, string methodName) = ResolveCallerLuaTarget();
            LuaAppDomain.RunLuaFunc(moduleName, methodName, args);
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static T Run<T>(params object[] args)
        {
            (string moduleName, string methodName) = ResolveCallerLuaTarget();
            return LuaAppDomain.RunLuaFunc<T>(moduleName, methodName, args);
        }

        [MethodImpl(MethodImplOptions.NoInlining)]
        private static (string moduleName, string methodName) ResolveCallerLuaTarget()
        {
            MethodBase caller = new StackTrace().GetFrame(2)?.GetMethod();
            if (caller == null)
            {
                throw new InvalidOperationException("Cannot resolve caller method for LuaInvoke.");
            }

            LuaInvokeAttribute attr = caller.GetCustomAttribute<LuaInvokeAttribute>();
            if (attr == null)
            {
                throw new InvalidOperationException($"Method '{caller.DeclaringType?.FullName}.{caller.Name}' is not marked with [LuaInvoke].");
            }

            if (string.IsNullOrWhiteSpace(attr.Module) || string.IsNullOrWhiteSpace(attr.Function))
            {
                throw new InvalidOperationException("[LuaInvoke] requires module and function names in editor mode.");
            }

            return (attr.Module, attr.Function);
        }
    }
}
