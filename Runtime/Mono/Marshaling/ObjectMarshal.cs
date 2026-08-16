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
using ZLua.Utils;

namespace ZLua.Marshaling
{
    internal static class ObjectMarshal
    {
        internal static void Push(IntPtr L, object obj, Type viewType)
        {
            if (obj == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            if (viewType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: viewType is null");
            }

            ObjectRegistry.Push(L, obj, viewType, LuaConsts.LuaNoRef);
        }

        internal static void Push(IntPtr L, object obj)
        {
            if (obj == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            ObjectRegistry.Push(L, obj, obj.GetType(), LuaConsts.LuaNoRef);
        }

        internal static object Pop(IntPtr L, int objIndex, Type declaredType)
        {
            if (declaredType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: declaredType is null");
            }

            LuaDataType type = LuaDll.lua_type(L, objIndex);
            switch (type)
            {
                case LuaDataType.Nil:
                    return null;

                case LuaDataType.UserData:
                {
                    if (declaredType.IsArray)
                    {
                        if (declaredType.GetArrayRank() == 1)
                        {
                            return ArrayMarshal.PopSzArray(L, objIndex, declaredType);
                        }
                    }

                    object obj = ObjectRegistry.Pop(L, objIndex);
                    if (obj != null && !declaredType.IsInstanceOfType(obj) && !IsAssignableRuntime(declaredType, obj.GetType()))
                    {
                        LuaCallbackBoundary.Throw(
                            $"zlua argument mismatch: object is not of type: {declaredType.FullName}");
                    }

                    return obj;
                }

                case LuaDataType.Boolean:
                {
                    if (!declaredType.IsAssignableFrom(typeof(bool)))
                    {
                        LuaCallbackBoundary.Throw("zlua argument mismatch: boolean value can only be assigned to boolean type");
                    }

                    return LuaDll.lua_toboolean(L, objIndex) != 0;
                }

                case LuaDataType.Number:
                {
                    if (LuaDll.lua_isinteger(L, objIndex) != 0)
                    {
                        long wide = LuaDll.lua_tointeger(L, objIndex);
                        if (wide >= int.MinValue && wide <= int.MaxValue)
                        {
                            if (!declaredType.IsAssignableFrom(typeof(int)))
                            {
                                LuaCallbackBoundary.Throw(
                                    "zlua argument mismatch: number value can only be assigned to int32 type");
                            }

                            return (int)wide;
                        }

                        if (!declaredType.IsAssignableFrom(typeof(long)))
                        {
                            LuaCallbackBoundary.Throw(
                                "zlua argument mismatch: integer out of int32 range requires int64-compatible target");
                        }

                        return wide;
                    }

                    if (!declaredType.IsAssignableFrom(typeof(double)))
                    {
                        LuaCallbackBoundary.Throw("zlua argument mismatch: number value can only be assigned to double type");
                    }

                    return LuaDll.lua_tonumber(L, objIndex);
                }

                case LuaDataType.String:
                {
                    if (!declaredType.IsAssignableFrom(typeof(string)))
                    {
                        LuaCallbackBoundary.Throw("zlua argument mismatch: string value can only be assigned to string type");
                    }

                    return LuaDllExtension.tostring(L, objIndex);
                }

                case LuaDataType.Table:
                {
                    if (declaredType.IsArray)
                    {
                        if (declaredType.GetArrayRank() == 1)
                        {
                            return ArrayMarshal.PopSzArray(L, objIndex, declaredType);
                        }

                        LuaCallbackBoundary.Throw("zlua not supported: mdarray cannot be popped from a table");
                    }

                    LuaCallbackBoundary.Throw("zlua argument mismatch: table value can only be assigned to array type");
                    return null;
                }

                case LuaDataType.Function:
                {
                    if (!typeof(Delegate).IsAssignableFrom(declaredType))
                    {
                        LuaCallbackBoundary.Throw("zlua argument mismatch: function value can only be assigned to delegate type");
                    }

                    return DelegateMarshal.Pop(L, objIndex, declaredType);
                }

                default:
                    LuaCallbackBoundary.Throw($"zlua argument mismatch: unsupported object type: {type}");
                    return null;
            }
        }

        private static bool IsAssignableRuntime(Type declaredType, Type runtimeType)
        {
            return declaredType.IsAssignableFrom(runtimeType);
        }
    }
}
