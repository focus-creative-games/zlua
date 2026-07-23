using System;
using System.Reflection;
using ZLua.Marshaling;
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

        internal static readonly MethodInfo StructWriteBack =
            Require(typeof(StructMarshal), nameof(StructMarshal.WriteBack));

        internal static readonly MethodInfo PushConstructorInstance =
            Require(typeof(TypeRegistry), nameof(TypeRegistry.PushConstructorInstance));

        internal static readonly MethodInfo PointerInvoke =
            Require(typeof(PointerMethodInvoker), nameof(PointerMethodInvoker.Invoke));

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
