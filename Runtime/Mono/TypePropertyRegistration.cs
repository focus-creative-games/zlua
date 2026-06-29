using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    internal static class TypePropertyRegistration
    {
        private readonly struct PropertyKey : IEquatable<PropertyKey>
        {
            internal readonly int TypeId;
            internal readonly string Name;
            internal readonly bool IsStatic;

            internal PropertyKey(int typeId, string name, bool isStatic)
            {
                TypeId = typeId;
                Name = name;
                IsStatic = isStatic;
            }

            public bool Equals(PropertyKey other)
            {
                return TypeId == other.TypeId
                    && IsStatic == other.IsStatic
                    && string.Equals(Name, other.Name, StringComparison.Ordinal);
            }

            public override bool Equals(object obj)
            {
                return obj is PropertyKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    int hash = TypeId;
                    hash = (hash * 397) ^ (IsStatic ? 1 : 0);
                    hash = (hash * 397) ^ (Name?.GetHashCode() ?? 0);
                    return hash;
                }
            }
        }

        private static readonly Dictionary<int, PropertyInfo> StaticProperties = new Dictionary<int, PropertyInfo>();
        private static readonly Dictionary<int, PropertyInfo> InstanceProperties = new Dictionary<int, PropertyInfo>();
        private static readonly Dictionary<int, LuaCSFunction> StaticPropertyGetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> StaticPropertySetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> InstancePropertyGetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> InstancePropertySetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<PropertyKey, int> StaticPropertyIdsByKey = new Dictionary<PropertyKey, int>();
        private static readonly Dictionary<PropertyKey, int> InstancePropertyIdsByKey = new Dictionary<PropertyKey, int>();
        private static readonly List<LuaCSFunction> CallbackRefs = new List<LuaCSFunction>();

        private static int _nextPropertyId = 1;

        internal static int TryInvokeInstanceGetter(IntPtr luaState, int typeId, string propertyName)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                return -1;
            }

            PropertyKey key = new PropertyKey(typeId, propertyName, isStatic: false);
            if (!InstancePropertyIdsByKey.TryGetValue(key, out int propertyId))
            {
                return -1;
            }

            if (InstancePropertyGetters.TryGetValue(propertyId, out LuaCSFunction getter))
            {
                return getter(luaState);
            }

            return -1;
        }

        internal static bool IsInstancePropertyGetter(int typeId, string propertyName)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                return false;
            }

            PropertyKey key = new PropertyKey(typeId, propertyName, isStatic: false);
            return InstancePropertyIdsByKey.TryGetValue(key, out int propertyId)
                && InstancePropertyGetters.ContainsKey(propertyId);
        }

        internal static void BindGetterTable(IntPtr luaState, int getterTableIndex, int typeId, bool isStatic)
        {
            int absTableIndex = LuaDll.lua_absindex(luaState, getterTableIndex);
            Dictionary<PropertyKey, int> idMap = isStatic ? StaticPropertyIdsByKey : InstancePropertyIdsByKey;
            Dictionary<int, LuaCSFunction> getters = isStatic ? StaticPropertyGetters : InstancePropertyGetters;
            Dictionary<int, PropertyInfo> properties = isStatic ? StaticProperties : InstanceProperties;

            foreach (KeyValuePair<PropertyKey, int> entry in idMap)
            {
                PropertyKey key = entry.Key;
                if (key.TypeId != typeId || key.IsStatic != isStatic)
                {
                    continue;
                }

                int propertyId = entry.Value;
                if (!properties.TryGetValue(propertyId, out PropertyInfo property)
                    || property.GetGetMethod(nonPublic: false) == null)
                {
                    continue;
                }

                if (getters.TryGetValue(propertyId, out LuaCSFunction getter))
                {
                    TypeMethodRegistration.PushCompiledBridge(luaState, getter);
                }
                else
                {
                    PushFallbackGetterClosure(luaState, propertyId, isStatic);
                }

                LuaDll.lua_setfield(luaState, absTableIndex, key.Name);
            }
        }

        internal static void BindSetterTable(IntPtr luaState, int setterTableIndex, int typeId, bool isStatic)
        {
            int absTableIndex = LuaDll.lua_absindex(luaState, setterTableIndex);
            Dictionary<PropertyKey, int> idMap = isStatic ? StaticPropertyIdsByKey : InstancePropertyIdsByKey;
            Dictionary<int, LuaCSFunction> setters = isStatic ? StaticPropertySetters : InstancePropertySetters;
            Dictionary<int, PropertyInfo> properties = isStatic ? StaticProperties : InstanceProperties;

            foreach (KeyValuePair<PropertyKey, int> entry in idMap)
            {
                PropertyKey key = entry.Key;
                if (key.TypeId != typeId || key.IsStatic != isStatic)
                {
                    continue;
                }

                int propertyId = entry.Value;
                if (!properties.TryGetValue(propertyId, out PropertyInfo property)
                    || property.GetSetMethod(nonPublic: false) == null)
                {
                    continue;
                }

                if (setters.TryGetValue(propertyId, out LuaCSFunction setter))
                {
                    TypeMethodRegistration.PushCompiledBridge(luaState, setter);
                }
                else
                {
                    PushFallbackSetterClosure(luaState, propertyId, isStatic);
                }

                LuaDll.lua_setfield(luaState, absTableIndex, key.Name);
            }
        }

        private static void PushFallbackGetterClosure(IntPtr luaState, int propertyId, bool isStatic)
        {
            LuaCSFunction callback = isStatic ? StaticPropertyGetterFallbackCallback : InstancePropertyGetterFallbackCallback;
            CallbackRefs.Add(callback);
            IntPtr functionPointer = System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(callback);
            LuaDll.lua_pushinteger(luaState, propertyId);
            LuaDll.lua_pushcclosure(luaState, functionPointer, 1);
        }

        private static void PushFallbackSetterClosure(IntPtr luaState, int propertyId, bool isStatic)
        {
            LuaCSFunction callback = isStatic ? StaticPropertySetterFallbackCallback : InstancePropertySetterFallbackCallback;
            CallbackRefs.Add(callback);
            IntPtr functionPointer = System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(callback);
            LuaDll.lua_pushinteger(luaState, propertyId);
            LuaDll.lua_pushcclosure(luaState, functionPointer, 1);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstancePropertyGetterFallback(IntPtr luaState)
        {
            int propertyId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!InstanceProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                return LuaDllExtension.error(luaState, "zlua: internal property getter not found");
            }

            try
            {
                TypePropertyRegistrationFallback.PushInstanceProperty(luaState, property);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticPropertyGetterFallback(IntPtr luaState)
        {
            int propertyId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!StaticProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                return LuaDllExtension.error(luaState, "zlua: internal property getter not found");
            }

            try
            {
                TypePropertyRegistrationFallback.PushStaticProperty(luaState, property);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstancePropertySetterFallback(IntPtr luaState)
        {
            int propertyId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!InstanceProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                return LuaDllExtension.error(luaState, "zlua: internal property setter not found");
            }

            return TypePropertyRegistrationFallback.AssignInstanceProperty(luaState, property);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticPropertySetterFallback(IntPtr luaState)
        {
            int propertyId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!StaticProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                return LuaDllExtension.error(luaState, "zlua: internal property setter not found");
            }

            return TypePropertyRegistrationFallback.AssignStaticProperty(luaState, property);
        }

        private static readonly LuaCSFunction InstancePropertyGetterFallbackCallback = InvokeInstancePropertyGetterFallback;
        private static readonly LuaCSFunction StaticPropertyGetterFallbackCallback = InvokeStaticPropertyGetterFallback;
        private static readonly LuaCSFunction InstancePropertySetterFallbackCallback = InvokeInstancePropertySetterFallback;
        private static readonly LuaCSFunction StaticPropertySetterFallbackCallback = InvokeStaticPropertySetterFallback;

        internal static void RegisterProperties(Type type, int typeId)
        {
            if (type == null || type.IsEnum)
            {
                return;
            }

            List<PropertyInfo> staticProperties = CollectProperties(type, isStatic: true);
            for (int i = 0; i < staticProperties.Count; i++)
            {
                RegisterProperty(staticProperties[i], typeId, isStatic: true);
            }

            List<PropertyInfo> instanceProperties = CollectProperties(type, isStatic: false);
            for (int i = 0; i < instanceProperties.Count; i++)
            {
                RegisterProperty(instanceProperties[i], typeId, isStatic: false);
            }
        }

        internal static int TryGetStaticProperty(IntPtr luaState, int typeId, string propertyName)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                return -1;
            }

            PropertyKey key = new PropertyKey(typeId, propertyName, isStatic: true);
            if (!StaticPropertyIdsByKey.TryGetValue(key, out int propertyId))
            {
                return -1;
            }

            if (StaticPropertyGetters.TryGetValue(propertyId, out LuaCSFunction getter))
            {
                return getter(luaState);
            }

            if (StaticProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                TypePropertyRegistrationFallback.PushStaticProperty(luaState, property);
                return 1;
            }

            return -1;
        }

        internal static int TrySetStaticProperty(IntPtr luaState, int typeId, string propertyName)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                return -1;
            }

            PropertyKey key = new PropertyKey(typeId, propertyName, isStatic: true);
            if (!StaticPropertyIdsByKey.TryGetValue(key, out int propertyId))
            {
                return -1;
            }

            if (StaticPropertySetters.TryGetValue(propertyId, out LuaCSFunction setter))
            {
                return setter(luaState);
            }

            if (StaticProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                return TypePropertyRegistrationFallback.AssignStaticProperty(luaState, property);
            }

            return -1;
        }

        internal static int TryGetInstanceProperty(IntPtr luaState, int typeId, string propertyName)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                return -1;
            }

            PropertyKey key = new PropertyKey(typeId, propertyName, isStatic: false);
            if (!InstancePropertyIdsByKey.TryGetValue(key, out int propertyId))
            {
                return -1;
            }

            if (InstancePropertyGetters.TryGetValue(propertyId, out LuaCSFunction getter))
            {
                return getter(luaState);
            }

            if (InstanceProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                TypePropertyRegistrationFallback.PushInstanceProperty(luaState, property);
                return 1;
            }

            return -1;
        }

        internal static int TrySetInstanceProperty(IntPtr luaState, int typeId, string propertyName)
        {
            if (string.IsNullOrEmpty(propertyName))
            {
                return -1;
            }

            PropertyKey key = new PropertyKey(typeId, propertyName, isStatic: false);
            if (!InstancePropertyIdsByKey.TryGetValue(key, out int propertyId))
            {
                return -1;
            }

            if (InstancePropertySetters.TryGetValue(propertyId, out LuaCSFunction setter))
            {
                return setter(luaState);
            }

            if (InstanceProperties.TryGetValue(propertyId, out PropertyInfo property))
            {
                return TypePropertyRegistrationFallback.AssignInstanceProperty(luaState, property);
            }

            return -1;
        }

        private static void RegisterProperty(PropertyInfo property, int typeId, bool isStatic)
        {
            PropertyKey key = new PropertyKey(typeId, property.Name, isStatic);
            Dictionary<PropertyKey, int> idMap = isStatic ? StaticPropertyIdsByKey : InstancePropertyIdsByKey;
            if (idMap.ContainsKey(key))
            {
                return;
            }

            int propertyId = _nextPropertyId++;
            idMap[key] = propertyId;

            if (isStatic)
            {
                StaticProperties[propertyId] = property;
                if (LuaToCSharpPropertyBridgeFactory.TryCreateGetter(property, isStatic: true, out LuaCSFunction getter))
                {
                    StaticPropertyGetters[propertyId] = getter;
                    CallbackRefs.Add(getter);
                }

                if (LuaToCSharpPropertyBridgeFactory.TryCreateSetter(property, isStatic: true, out LuaCSFunction setter))
                {
                    StaticPropertySetters[propertyId] = setter;
                    CallbackRefs.Add(setter);
                }
            }
            else
            {
                InstanceProperties[propertyId] = property;
                if (LuaToCSharpPropertyBridgeFactory.TryCreateGetter(property, isStatic: false, out LuaCSFunction getter))
                {
                    InstancePropertyGetters[propertyId] = getter;
                    CallbackRefs.Add(getter);
                }

                if (LuaToCSharpPropertyBridgeFactory.TryCreateSetter(property, isStatic: false, out LuaCSFunction setter))
                {
                    InstancePropertySetters[propertyId] = setter;
                    CallbackRefs.Add(setter);
                }
            }
        }

        private static List<PropertyInfo> CollectProperties(Type type, bool isStatic)
        {
            BindingFlags flags = (isStatic ? BindingFlags.Static : BindingFlags.Instance)
                | BindingFlags.Public
                | BindingFlags.DeclaredOnly;

            Dictionary<string, PropertyInfo> byName = new Dictionary<string, PropertyInfo>(StringComparer.Ordinal);
            for (Type current = type; current != null; current = current.BaseType)
            {
                PropertyInfo[] properties = current.GetProperties(flags);
                for (int i = 0; i < properties.Length; i++)
                {
                    PropertyInfo property = properties[i];
                    if (property.GetIndexParameters().Length != 0)
                    {
                        continue;
                    }

                    if (!byName.ContainsKey(property.Name))
                    {
                        byName[property.Name] = property;
                    }
                }
            }

            List<PropertyInfo> result = new List<PropertyInfo>(byName.Count);
            foreach (KeyValuePair<string, PropertyInfo> kv in byName)
            {
                result.Add(kv.Value);
            }

            return result;
        }
    }

    internal static class TypePropertyRegistrationFallback
    {
        internal static void PushStaticProperty(IntPtr luaState, PropertyInfo property)
        {
            MethodInfo getter = property.GetGetMethod(nonPublic: false);
            if (getter == null)
            {
                return;
            }

            object value = getter.Invoke(null, null);
            TypeMethodRegistration.PushReturnDefaultPublic(luaState, property.PropertyType, value);
        }

        internal static void SetStaticProperty(IntPtr luaState, PropertyInfo property)
        {
            AssignStaticProperty(luaState, property);
        }

        internal static int AssignStaticProperty(IntPtr luaState, PropertyInfo property)
        {
            MethodInfo setter = property.GetSetMethod(nonPublic: false);
            if (setter == null)
            {
                return LuaDllExtension.error(luaState, $"zlua: property is read-only: {property.Name}");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadValuePublic(luaState, 3, property.PropertyType);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            try
            {
                setter.Invoke(null, new[] { value });
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            return 0;
        }

        internal static void PushInstanceProperty(IntPtr luaState, PropertyInfo property)
        {
            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, 1, out object target))
            {
                LuaCallbackBoundary.Throw("zlua: invalid userdata for member access");
            }

            MethodInfo getter = property.GetGetMethod(nonPublic: false);
            if (getter == null)
            {
                return;
            }

            object value = getter.Invoke(target, null);
            TypeMethodRegistration.PushReturnDefaultPublic(luaState, property.PropertyType, value);
        }

        internal static void SetInstanceProperty(IntPtr luaState, PropertyInfo property)
        {
            AssignInstanceProperty(luaState, property);
        }

        internal static int AssignInstanceProperty(IntPtr luaState, PropertyInfo property)
        {
            MethodInfo setter = property.GetSetMethod(nonPublic: false);
            if (setter == null)
            {
                return LuaDllExtension.error(luaState, $"zlua: property is read-only: {property.Name}");
            }

            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, 1, out object target))
            {
                return LuaDllExtension.error(luaState, "zlua: invalid userdata for property assignment");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadValuePublic(luaState, 3, property.PropertyType);
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
    }
}
