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
                        if (!declaredType.IsAssignableFrom(typeof(int)))
                        {
                            LuaCallbackBoundary.Throw("zlua argument mismatch: number value can only be assigned to int32 type");
                        }

                        return (int)LuaDll.lua_tointeger(L, objIndex);
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
