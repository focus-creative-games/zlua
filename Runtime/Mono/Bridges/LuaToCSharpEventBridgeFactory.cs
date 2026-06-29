using System;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// Compiles Lua → C# event add/remove/raise bridges for default-marshal public events.
    /// </summary>
    internal static class LuaToCSharpEventBridgeFactory
    {
        internal static bool TryCreateAdd(EventInfo eventInfo, bool isStatic, out LuaCSFunction invoker)
        {
            return TryCreateAccessor(eventInfo?.GetAddMethod(nonPublic: false), isStatic, out invoker);
        }

        internal static bool TryCreateRemove(EventInfo eventInfo, bool isStatic, out LuaCSFunction invoker)
        {
            return TryCreateAccessor(eventInfo?.GetRemoveMethod(nonPublic: false), isStatic, out invoker);
        }

        internal static bool TryCreateFire(EventInfo eventInfo, MethodInfo raiseMethod, bool isStatic, out LuaCSFunction invoker)
        {
            invoker = null;
            if (eventInfo == null || raiseMethod == null || !CanFastEvent(eventInfo, isStatic))
            {
                return false;
            }

            return LuaToCSharpMethodBridgeFactory.TryCreate(raiseMethod, isStatic, out invoker);
        }

        private static bool TryCreateAccessor(MethodInfo accessor, bool isStatic, out LuaCSFunction invoker)
        {
            invoker = null;
            if (accessor == null || accessor.IsStatic != isStatic)
            {
                return false;
            }

            if (accessor.GetParameters().Length != 1)
            {
                return false;
            }

            return LuaToCSharpMethodBridgeFactory.TryCreate(accessor, isStatic, out invoker);
        }

        internal static bool CanFastEvent(EventInfo eventInfo, bool isStatic)
        {
            if (eventInfo == null || eventInfo.DeclaringType == null || eventInfo.DeclaringType.ContainsGenericParameters)
            {
                return false;
            }

            if (eventInfo.IsDefined(typeof(LuaMarshalAsAttribute), inherit: false))
            {
                return false;
            }

            Type handlerType = eventInfo.EventHandlerType;
            if (handlerType == null || !typeof(Delegate).IsAssignableFrom(handlerType))
            {
                return false;
            }

            MethodInfo invoke = handlerType.GetMethod("Invoke");
            if (invoke == null)
            {
                return false;
            }

            ParameterInfo[] invokeParameters = invoke.GetParameters();
            for (int i = 0; i < invokeParameters.Length; i++)
            {
                if (!IsFastType(invokeParameters[i].ParameterType))
                {
                    return false;
                }
            }

            if (invoke.ReturnType != typeof(void) && !IsFastType(invoke.ReturnType))
            {
                return false;
            }

            MethodInfo addMethod = eventInfo.GetAddMethod(nonPublic: false);
            if (addMethod == null || addMethod.IsStatic != isStatic)
            {
                return false;
            }

            MethodInfo removeMethod = eventInfo.GetRemoveMethod(nonPublic: false);
            if (removeMethod == null || removeMethod.IsStatic != isStatic)
            {
                return false;
            }

            return LuaToCSharpMethodBridgeFactory.CanFastInvoke(addMethod, isStatic)
                && LuaToCSharpMethodBridgeFactory.CanFastInvoke(removeMethod, isStatic);
        }

        private static bool IsFastType(Type type)
        {
            if (Nullable.GetUnderlyingType(type) != null)
            {
                return false;
            }

            Type targetType = type;
            if (targetType.IsByRef || targetType == typeof(object) || targetType == typeof(TypedReference))
            {
                return false;
            }

            if (targetType.IsPrimitive
                || targetType == typeof(string)
                || targetType == typeof(decimal)
                || targetType == typeof(IntPtr)
                || targetType == typeof(UIntPtr))
            {
                return true;
            }

            if (targetType.IsEnum || ValueTypeMarshaling.IsStructType(targetType))
            {
                return true;
            }

            if (targetType.IsArray)
            {
                return true;
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                return true;
            }

            return targetType.IsClass || targetType.IsInterface;
        }
    }
}
