// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
