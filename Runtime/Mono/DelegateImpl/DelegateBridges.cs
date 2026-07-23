using System;

namespace ZLua.DelegateImpl
{
    /// <summary>
    /// Pre-compiles common Lua→C# delegate bridges on startup.
    /// </summary>
    internal static class DelegateBridges
    {
        internal static void Warmup()
        {
            DynamicBridgeFactory.Warmup(typeof(Action));
            DynamicBridgeFactory.Warmup(typeof(Action<int>));
            DynamicBridgeFactory.Warmup(typeof(Func<int, int>));
        }
    }
}
