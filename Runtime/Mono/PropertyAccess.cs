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

            try
            {
                object value = getter.Invoke(target, null);
                return TypeMethodRegistration.PushReturnValue(luaState, property.PropertyType, value);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua property get error: {ex.InnerException?.Message ?? ex.Message}");
            }
        }

        internal static int SetPropertyValue(IntPtr luaState, PropertyInfo property, object target, int valueIndex)
        {
            MethodInfo setter = property.GetSetMethod(false);
            if (setter == null)
            {
                return LuaDllExtension.error(luaState, $"zlua: property is read-only: {property.Name}");
            }

            try
            {
                object value = TypeMethodRegistration.ReadArgumentValue(luaState, valueIndex, property.PropertyType);
                setter.Invoke(target, new[] { value });
                return 0;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua property set error: {ex.Message}");
            }
        }

        private static bool IsParameterlessProperty(PropertyInfo property)
        {
            return property != null && property.GetIndexParameters().Length == 0;
        }
    }
}
