using System;

namespace ZLua
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public sealed class LuaInvokeAttribute : Attribute
    {
        public string Module { get; }

        public string Function { get; }

        public LuaInvokeAttribute()
        {
        }

        public LuaInvokeAttribute(string module, string function)
        {
            Module = module;
            Function = function;
        }
    }
}
