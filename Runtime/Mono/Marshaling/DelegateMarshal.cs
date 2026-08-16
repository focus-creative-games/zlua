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
using ZLua;
using ZLua.DelegateImpl;
using ZLua.Lvm;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class DelegateMarshal
    {
        internal static void Push(IntPtr L, Delegate del, Type viewType = null)
        {
            if (del == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            if (del.Target is LuaMethod luaMethod)
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, luaMethod.RefIndex);
                return;
            }

            ObjectMarshal.Push(L, del, viewType ?? del.GetType());
        }

        internal static Delegate Pop(IntPtr L, int index, Type delegateType)
        {
            if (delegateType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: delegateType is null");
            }

            if (!typeof(Delegate).IsAssignableFrom(delegateType))
            {
                LuaCallbackBoundary.Throw("zlua: argument mismatch: expected delegate");
            }

            LuaDataType valueType = LuaDll.lua_type(L, index);
            switch (valueType)
            {
                case LuaDataType.Function:
                {
                    LuaEnv env = LuaEnv.Active;
                    if (env == null)
                    {
                        LuaCallbackBoundary.Throw("zlua internal error: no active LuaEnv");
                    }

                    int funcRef = LuaDelegateBinder.CreateFunctionRef(L, index);
                    return LuaDelegateBinder.Create(env, delegateType, funcRef);
                }

                case LuaDataType.UserData:
                {
                    object obj = ObjectRegistry.Pop(L, index);
                    if (obj == null || !delegateType.IsInstanceOfType(obj))
                    {
                        LuaCallbackBoundary.Throw("zlua: argument mismatch: expected delegate");
                    }

                    return (Delegate)obj;
                }

                case LuaDataType.Nil:
                    return null;

                default:
                    LuaCallbackBoundary.Throw("zlua: argument mismatch: expected delegate");
                    return null;
            }
        }
    }
}
