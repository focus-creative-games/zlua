using System;
using System.Reflection;
using ZLua.Mt;

namespace ZLua.Emit
{
    /// <summary>
    /// Cached MethodInfo for Expression trees (internal helpers need NonPublic bind flags).
    /// </summary>
    internal static class EmitMethods
    {
        private const BindingFlags StaticAny =
            BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;

        internal static readonly MethodInfo ValidateExactArgCount =
            Require(typeof(BridgeMarshaling), nameof(BridgeMarshaling.ValidateExactArgCount));

        internal static readonly MethodInfo PopTarget =
            Require(typeof(BridgeMarshaling), nameof(BridgeMarshaling.PopTarget));

        internal static readonly MethodInfo PopArg =
            Require(typeof(BridgeMarshaling), nameof(BridgeMarshaling.PopArg));

        internal static readonly MethodInfo PushReturn =
            Require(typeof(BridgeMarshaling), nameof(BridgeMarshaling.PushReturn));

        internal static readonly MethodInfo PushConstructorInstance =
            Require(typeof(TypeRegistry), nameof(TypeRegistry.PushConstructorInstance));

        internal static readonly MethodInfo PointerInvoke =
            Require(typeof(PointerMethodInvoker), nameof(PointerMethodInvoker.Invoke));

        internal static readonly MethodInfo ByValGetField =
            Require(typeof(ByValInstanceOps), nameof(ByValInstanceOps.GetField));

        internal static readonly MethodInfo ByValSetField =
            Require(typeof(ByValInstanceOps), nameof(ByValInstanceOps.SetField));

        internal static readonly MethodInfo ByValInvokeInstance =
            Require(typeof(ByValInstanceOps), nameof(ByValInstanceOps.InvokeInstance));

        internal static readonly MethodInfo ByValInvokePropertyGetter =
            Require(typeof(ByValInstanceOps), nameof(ByValInstanceOps.InvokePropertyGetter));

        internal static readonly MethodInfo ByValInvokePropertySetter =
            Require(typeof(ByValInstanceOps), nameof(ByValInstanceOps.InvokePropertySetter));

        private static MethodInfo Require(Type type, string name)
        {
            MethodInfo method = type.GetMethod(name, StaticAny);
            if (method == null)
            {
                throw new InvalidOperationException($"zlua emit: missing method {type.FullName}.{name}");
            }

            return method;
        }
    }
}
