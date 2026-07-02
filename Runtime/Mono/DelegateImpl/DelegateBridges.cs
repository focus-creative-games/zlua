using System;
using ZLua;
using ZLua.Marshal;
using ZLua.MethodBridge;
using ZLua.Mt;

namespace ZLua.DelegateImpl
{
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
