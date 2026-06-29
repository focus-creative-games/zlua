using System;

namespace ZLua
{
    /// <summary>
    /// Injected by <c>LuaInvokeWeaver</c> after processing an assembly (not applied in source).
    /// </summary>
    [AttributeUsage(AttributeTargets.Assembly, AllowMultiple = false)]
    internal sealed class LuaInvokeWeaverProcessedAttribute : Attribute
    {
        internal const int CurrentWeaveVersion = 4;

        public int WeaveVersion { get; }

        public LuaInvokeWeaverProcessedAttribute(int weaveVersion)
        {
            WeaveVersion = weaveVersion;
        }
    }
}
