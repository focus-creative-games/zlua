using System;

namespace ZLua
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
    public sealed class LuaAliasAttribute : Attribute
    {
        public string Alias { get; }

        public LuaAliasAttribute(string alias)
        {
            Alias = alias;
        }
    }
}
