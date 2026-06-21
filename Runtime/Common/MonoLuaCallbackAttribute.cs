using System;

namespace NovaLua
{
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public sealed class MonoLuaCallbackAttribute : Attribute
    {
        public Type DelegateType { get; }

        public MonoLuaCallbackAttribute(Type delegateType)
        {
            DelegateType = delegateType;
        }
    }
}
