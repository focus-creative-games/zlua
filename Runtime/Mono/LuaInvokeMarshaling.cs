using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// C# → Lua (<see cref="LuaInvoke"/>) push/pop with <see cref="LuaMarshalAsAttribute"/> resolution.
    /// </summary>
    internal static class LuaInvokeMarshaling
    {
        internal static void PushArgument(
            IntPtr luaState,
            object value,
            ParameterInfo[] parameters,
            MethodInfo invokeMethod,
            int parameterIndex)
        {
            if (invokeMethod == null || parameters == null || parameterIndex < 0 || parameterIndex >= parameters.Length)
            {
                if (value == null)
                {
                    LuaDll.lua_pushnil(luaState);
                    return;
                }

                LuaMarshal.PushObject(luaState, value);
                return;
            }

            ParameterInfo parameter = parameters[parameterIndex];
            Type parameterType = parameter.ParameterType;
            LuaMarshalType marshalType = LuaMarshalAsResolver.ResolveParameter(
                parameter,
                invokeMethod,
                LuaMarshalDirection.CSharpToLua,
                parameterType);

            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            if (marshalType != LuaMarshalType.Default)
            {
                LuaMarshalDispatch.PushArgument(luaState, value, parameterType, marshalType);
                return;
            }

            LuaMarshal.PushObject(luaState, value);
        }

        internal static object PopReturn(IntPtr luaState, MethodInfo invokeMethod, Type returnType, int luaIndex)
        {
            if (returnType == typeof(void))
            {
                return null;
            }

            LuaMarshalType marshalType = invokeMethod != null
                ? LuaMarshalAsResolver.ResolveReturn(invokeMethod, LuaMarshalDirection.LuaToCSharp)
                : LuaMarshalType.Default;

            if (marshalType != LuaMarshalType.Default)
            {
                if (!LuaMarshalDispatch.CanConvert(luaState, luaIndex, returnType, marshalType))
                {
                    throw new InvalidOperationException(
                        $"Lua return value cannot convert to {returnType.FullName} with LuaMarshalType.{marshalType}.");
                }

                return LuaMarshalDispatch.Read(luaState, luaIndex, returnType, marshalType);
            }

            return LuaMarshal.PopObject(luaState, returnType, luaIndex);
        }
    }
}
