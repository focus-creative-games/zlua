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
using ZLua;
using ZLua.Lvm;

namespace ZLua.DelegateImpl
{
    internal static class LuaDelegateBinder
    {
        internal static Delegate Create(LuaEnv env, Type delegateType, int funcRef)
        {
            if (env == null)
            {
                throw new ArgumentNullException(nameof(env));
            }

            if (delegateType == null)
            {
                throw new ArgumentNullException(nameof(delegateType));
            }

            if (!typeof(Delegate).IsAssignableFrom(delegateType))
            {
                throw new ArgumentException($"Type '{delegateType.FullName}' is not a delegate type.", nameof(delegateType));
            }

            MethodInfo invokeMethod = delegateType.GetMethod("Invoke");
            if (invokeMethod == null)
            {
                throw new InvalidOperationException($"Delegate type '{delegateType.FullName}' has no Invoke method.");
            }

            LuaMethod target = new LuaMethod(env, funcRef);
            return DynamicBridgeFactory.CreateDelegate(delegateType, target);
        }

        internal static int CreateFunctionRef(IntPtr luaState, int index)
        {
            LuaDll.lua_pushvalue(luaState, index);
            int funcRef = LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
            if (funcRef < 0)
            {
                throw new InvalidOperationException("Failed to create lua function reference.");
            }

            return funcRef;
        }
    }
}
