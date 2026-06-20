using System;

namespace NextLua
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public sealed class LuaCallbackAttribute : Attribute
    {
    }
}
