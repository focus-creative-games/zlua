using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Runtime resolver; delegates to <see cref="LuaMarshalAsValidation"/> (MARSHAL_SPEC §6).
    /// </summary>
    internal static class LuaMarshalAsResolver
    {
        internal static LuaMarshalType ResolveParameter(
            ParameterInfo parameter,
            MethodBase method,
            LuaMarshalDirection direction,
            Type effectiveClrType = null)
        {
            return ResolveParameterBinding(parameter, method, direction, effectiveClrType).MarshalType;
        }

        internal static LuaMarshalType ResolveReturn(MethodInfo method, LuaMarshalDirection direction)
        {
            return ResolveReturnBinding(method, direction).MarshalType;
        }

        internal static LuaMarshalBinding ResolveParameterBinding(
            ParameterInfo parameter,
            MethodBase method,
            LuaMarshalDirection direction,
            Type effectiveClrType = null)
        {
            return LuaMarshalAsValidation.ResolveParameterBinding(parameter, method, direction, effectiveClrType);
        }

        internal static LuaMarshalBinding ResolveReturnBinding(MethodInfo method, LuaMarshalDirection direction)
        {
            return LuaMarshalAsValidation.ResolveReturnBinding(method, direction);
        }

        internal static void ValidateMethodConfiguration(MethodBase method)
        {
            LuaMarshalAsValidation.ValidateMethodConfiguration(method);
        }
    }
}
