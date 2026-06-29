using System;
using System.Reflection;

namespace ZLua
{
    internal static class PropertyAccess
    {
        internal static PropertyInfo FindInstanceProperty(Type type, string name)
        {
            for (Type current = type; current != null; current = current.BaseType)
            {
                PropertyInfo property = current.GetProperty(
                    name,
                    BindingFlags.Public | BindingFlags.Instance | BindingFlags.DeclaredOnly);
                if (IsParameterlessProperty(property))
                {
                    return property;
                }
            }

            return null;
        }

        internal static PropertyInfo FindStaticProperty(Type type, string name)
        {
            for (Type current = type; current != null; current = current.BaseType)
            {
                PropertyInfo property = current.GetProperty(
                    name,
                    BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly);
                if (IsParameterlessProperty(property))
                {
                    return property;
                }
            }

            return null;
        }

        internal static int PushPropertyGet(IntPtr luaState, PropertyInfo property, object target)
        {
            MethodInfo getter = property.GetGetMethod(false);
            if (getter == null)
            {
                return 0;
            }

            object value = getter.Invoke(target, null);
            return TypeMethodRegistration.PushReturnValue(luaState, property.PropertyType, value);
        }

        internal static int SetPropertyValue(IntPtr luaState, PropertyInfo property, object target, int valueIndex)
        {
            MethodInfo setter = property.GetSetMethod(false);
            if (setter == null)
            {
                return LuaDllExtension.error(luaState, $"zlua: property is read-only: {property.Name}");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadArgumentValue(luaState, valueIndex, property.PropertyType);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            try
            {
                setter.Invoke(target, new[] { value });
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            return 0;
        }

        private static bool IsParameterlessProperty(PropertyInfo property)
        {
            return property != null && property.GetIndexParameters().Length == 0;
        }
    }
}
