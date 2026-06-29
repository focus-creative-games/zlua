using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    internal static class TypeFieldRegistration
    {
        private readonly struct FieldKey : IEquatable<FieldKey>
        {
            internal readonly int TypeId;
            internal readonly string Name;
            internal readonly bool IsStatic;

            internal FieldKey(int typeId, string name, bool isStatic)
            {
                TypeId = typeId;
                Name = name;
                IsStatic = isStatic;
            }

            public bool Equals(FieldKey other)
            {
                return TypeId == other.TypeId
                    && IsStatic == other.IsStatic
                    && string.Equals(Name, other.Name, StringComparison.Ordinal);
            }

            public override bool Equals(object obj)
            {
                return obj is FieldKey other && Equals(other);
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

        private static readonly Dictionary<int, FieldInfo> StaticFields = new Dictionary<int, FieldInfo>();
        private static readonly Dictionary<int, FieldInfo> InstanceFields = new Dictionary<int, FieldInfo>();
        private static readonly Dictionary<int, LuaCSFunction> StaticFieldGetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> StaticFieldSetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> InstanceFieldGetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> InstanceFieldSetters = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<FieldKey, int> StaticFieldIdsByKey = new Dictionary<FieldKey, int>();
        private static readonly Dictionary<FieldKey, int> InstanceFieldIdsByKey = new Dictionary<FieldKey, int>();
        private static readonly List<LuaCSFunction> CallbackRefs = new List<LuaCSFunction>();

        private static int _nextFieldId = 1;

        internal static int TryInvokeInstanceGetter(IntPtr luaState, int typeId, string fieldName)
        {
            if (string.IsNullOrEmpty(fieldName))
            {
                return -1;
            }

            FieldKey key = new FieldKey(typeId, fieldName, isStatic: false);
            if (!InstanceFieldIdsByKey.TryGetValue(key, out int fieldId))
            {
                return -1;
            }

            if (InstanceFieldGetters.TryGetValue(fieldId, out LuaCSFunction getter))
            {
                return getter(luaState);
            }

            return -1;
        }

        internal static bool IsInstanceFieldGetter(int typeId, string fieldName)
        {
            if (string.IsNullOrEmpty(fieldName))
            {
                return false;
            }

            FieldKey key = new FieldKey(typeId, fieldName, isStatic: false);
            return InstanceFieldIdsByKey.TryGetValue(key, out int fieldId)
                && InstanceFieldGetters.ContainsKey(fieldId);
        }

        internal static void BindGetterTable(IntPtr luaState, int getterTableIndex, int typeId, bool isStatic)
        {
            int absTableIndex = LuaDll.lua_absindex(luaState, getterTableIndex);
            Dictionary<FieldKey, int> idMap = isStatic ? StaticFieldIdsByKey : InstanceFieldIdsByKey;
            Dictionary<int, LuaCSFunction> getters = isStatic ? StaticFieldGetters : InstanceFieldGetters;
            Dictionary<int, FieldInfo> fields = isStatic ? StaticFields : InstanceFields;

            foreach (KeyValuePair<FieldKey, int> entry in idMap)
            {
                FieldKey key = entry.Key;
                if (key.TypeId != typeId || key.IsStatic != isStatic)
                {
                    continue;
                }

                int fieldId = entry.Value;
                if (getters.TryGetValue(fieldId, out LuaCSFunction getter))
                {
                    TypeMethodRegistration.PushCompiledBridge(luaState, getter);
                }
                else if (fields.TryGetValue(fieldId, out FieldInfo field))
                {
                    PushFallbackGetterClosure(luaState, fieldId, isStatic);
                }
                else
                {
                    continue;
                }

                LuaDll.lua_setfield(luaState, absTableIndex, key.Name);
            }
        }

        internal static void BindSetterTable(IntPtr luaState, int setterTableIndex, int typeId, bool isStatic)
        {
            int absTableIndex = LuaDll.lua_absindex(luaState, setterTableIndex);
            Dictionary<FieldKey, int> idMap = isStatic ? StaticFieldIdsByKey : InstanceFieldIdsByKey;
            Dictionary<int, LuaCSFunction> setters = isStatic ? StaticFieldSetters : InstanceFieldSetters;
            Dictionary<int, FieldInfo> fields = isStatic ? StaticFields : InstanceFields;

            foreach (KeyValuePair<FieldKey, int> entry in idMap)
            {
                FieldKey key = entry.Key;
                if (key.TypeId != typeId || key.IsStatic != isStatic)
                {
                    continue;
                }

                int fieldId = entry.Value;
                if (setters.TryGetValue(fieldId, out LuaCSFunction setter))
                {
                    TypeMethodRegistration.PushCompiledBridge(luaState, setter);
                }
                else if (fields.TryGetValue(fieldId, out FieldInfo field)
                    && !field.IsInitOnly
                    && !field.IsLiteral)
                {
                    PushFallbackSetterClosure(luaState, fieldId, isStatic);
                }
                else
                {
                    continue;
                }

                LuaDll.lua_setfield(luaState, absTableIndex, key.Name);
            }
        }

        private static void PushFallbackGetterClosure(IntPtr luaState, int fieldId, bool isStatic)
        {
            LuaCSFunction callback = isStatic ? StaticFieldGetterFallbackCallback : InstanceFieldGetterFallbackCallback;
            CallbackRefs.Add(callback);
            IntPtr functionPointer = System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(callback);
            LuaDll.lua_pushinteger(luaState, fieldId);
            LuaDll.lua_pushcclosure(luaState, functionPointer, 1);
        }

        private static void PushFallbackSetterClosure(IntPtr luaState, int fieldId, bool isStatic)
        {
            LuaCSFunction callback = isStatic ? StaticFieldSetterFallbackCallback : InstanceFieldSetterFallbackCallback;
            CallbackRefs.Add(callback);
            IntPtr functionPointer = System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(callback);
            LuaDll.lua_pushinteger(luaState, fieldId);
            LuaDll.lua_pushcclosure(luaState, functionPointer, 1);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstanceFieldGetterFallback(IntPtr luaState)
        {
            int fieldId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!InstanceFields.TryGetValue(fieldId, out FieldInfo field))
            {
                return LuaDllExtension.error(luaState, "zlua: internal field getter not found");
            }

            try
            {
                TypeFieldRegistrationFallback.PushInstanceField(luaState, field);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticFieldGetterFallback(IntPtr luaState)
        {
            int fieldId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!StaticFields.TryGetValue(fieldId, out FieldInfo field))
            {
                return LuaDllExtension.error(luaState, "zlua: internal field getter not found");
            }

            try
            {
                TypeFieldRegistrationFallback.PushStaticField(luaState, field);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstanceFieldSetterFallback(IntPtr luaState)
        {
            int fieldId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!InstanceFields.TryGetValue(fieldId, out FieldInfo field))
            {
                return LuaDllExtension.error(luaState, "zlua: internal field setter not found");
            }

            return TypeFieldRegistrationFallback.AssignInstanceField(luaState, field);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticFieldSetterFallback(IntPtr luaState)
        {
            int fieldId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!StaticFields.TryGetValue(fieldId, out FieldInfo field))
            {
                return LuaDllExtension.error(luaState, "zlua: internal field setter not found");
            }

            return TypeFieldRegistrationFallback.AssignStaticField(luaState, field);
        }

        private static readonly LuaCSFunction InstanceFieldGetterFallbackCallback = InvokeInstanceFieldGetterFallback;
        private static readonly LuaCSFunction StaticFieldGetterFallbackCallback = InvokeStaticFieldGetterFallback;
        private static readonly LuaCSFunction InstanceFieldSetterFallbackCallback = InvokeInstanceFieldSetterFallback;
        private static readonly LuaCSFunction StaticFieldSetterFallbackCallback = InvokeStaticFieldSetterFallback;

        internal static void RegisterFields(Type type, int typeId)
        {
            if (type == null || type.IsEnum)
            {
                return;
            }

            List<FieldInfo> staticFields = CollectFields(type, isStatic: true);
            for (int i = 0; i < staticFields.Count; i++)
            {
                RegisterField(staticFields[i], typeId, isStatic: true);
            }

            List<FieldInfo> instanceFields = CollectFields(type, isStatic: false);
            for (int i = 0; i < instanceFields.Count; i++)
            {
                RegisterField(instanceFields[i], typeId, isStatic: false);
            }
        }

        internal static int TryGetStaticField(IntPtr luaState, int typeId, string fieldName)
        {
            if (string.IsNullOrEmpty(fieldName))
            {
                return -1;
            }

            FieldKey key = new FieldKey(typeId, fieldName, isStatic: true);
            if (!StaticFieldIdsByKey.TryGetValue(key, out int fieldId))
            {
                return -1;
            }

            if (StaticFieldGetters.TryGetValue(fieldId, out LuaCSFunction getter))
            {
                return getter(luaState);
            }

            if (StaticFields.TryGetValue(fieldId, out FieldInfo field))
            {
                TypeFieldRegistrationFallback.PushStaticField(luaState, field);
                return 1;
            }

            return -1;
        }

        internal static int TrySetStaticField(IntPtr luaState, int typeId, string fieldName)
        {
            if (string.IsNullOrEmpty(fieldName))
            {
                return -1;
            }

            FieldKey key = new FieldKey(typeId, fieldName, isStatic: true);
            if (!StaticFieldIdsByKey.TryGetValue(key, out int fieldId))
            {
                return -1;
            }

            if (StaticFieldSetters.TryGetValue(fieldId, out LuaCSFunction setter))
            {
                return setter(luaState);
            }

            if (StaticFields.TryGetValue(fieldId, out FieldInfo field))
            {
                return TypeFieldRegistrationFallback.AssignStaticField(luaState, field);
            }

            return -1;
        }

        internal static int TryGetInstanceField(IntPtr luaState, int typeId, string fieldName)
        {
            if (string.IsNullOrEmpty(fieldName))
            {
                return -1;
            }

            FieldKey key = new FieldKey(typeId, fieldName, isStatic: false);
            if (!InstanceFieldIdsByKey.TryGetValue(key, out int fieldId))
            {
                return -1;
            }

            if (InstanceFieldGetters.TryGetValue(fieldId, out LuaCSFunction getter))
            {
                return getter(luaState);
            }

            if (InstanceFields.TryGetValue(fieldId, out FieldInfo field))
            {
                TypeFieldRegistrationFallback.PushInstanceField(luaState, field);
                return 1;
            }

            return -1;
        }

        internal static int TrySetInstanceField(IntPtr luaState, int typeId, string fieldName)
        {
            if (string.IsNullOrEmpty(fieldName))
            {
                return -1;
            }

            FieldKey key = new FieldKey(typeId, fieldName, isStatic: false);
            if (!InstanceFieldIdsByKey.TryGetValue(key, out int fieldId))
            {
                return -1;
            }

            if (InstanceFieldSetters.TryGetValue(fieldId, out LuaCSFunction setter))
            {
                return setter(luaState);
            }

            if (InstanceFields.TryGetValue(fieldId, out FieldInfo field))
            {
                return TypeFieldRegistrationFallback.AssignInstanceField(luaState, field);
            }

            return -1;
        }

        private static void RegisterField(FieldInfo field, int typeId, bool isStatic)
        {
            FieldKey key = new FieldKey(typeId, field.Name, isStatic);
            Dictionary<FieldKey, int> idMap = isStatic ? StaticFieldIdsByKey : InstanceFieldIdsByKey;
            if (idMap.ContainsKey(key))
            {
                return;
            }

            int fieldId = _nextFieldId++;
            idMap[key] = fieldId;

            if (isStatic)
            {
                StaticFields[fieldId] = field;
                if (LuaToCSharpFieldBridgeFactory.TryCreateGetter(field, isStatic: true, out LuaCSFunction getter))
                {
                    StaticFieldGetters[fieldId] = getter;
                    CallbackRefs.Add(getter);
                }

                if (LuaToCSharpFieldBridgeFactory.TryCreateSetter(field, isStatic: true, out LuaCSFunction setter))
                {
                    StaticFieldSetters[fieldId] = setter;
                    CallbackRefs.Add(setter);
                }
            }
            else
            {
                InstanceFields[fieldId] = field;
                if (LuaToCSharpFieldBridgeFactory.TryCreateGetter(field, isStatic: false, out LuaCSFunction getter))
                {
                    InstanceFieldGetters[fieldId] = getter;
                    CallbackRefs.Add(getter);
                }

                if (LuaToCSharpFieldBridgeFactory.TryCreateSetter(field, isStatic: false, out LuaCSFunction setter))
                {
                    InstanceFieldSetters[fieldId] = setter;
                    CallbackRefs.Add(setter);
                }
            }
        }

        private static List<FieldInfo> CollectFields(Type type, bool isStatic)
        {
            BindingFlags flags = (isStatic ? BindingFlags.Static : BindingFlags.Instance)
                | BindingFlags.Public
                | BindingFlags.DeclaredOnly;

            Dictionary<string, FieldInfo> byName = new Dictionary<string, FieldInfo>(StringComparer.Ordinal);
            for (Type current = type; current != null; current = current.BaseType)
            {
                FieldInfo[] fields = current.GetFields(flags);
                for (int i = 0; i < fields.Length; i++)
                {
                    FieldInfo field = fields[i];
                    if (!byName.ContainsKey(field.Name))
                    {
                        byName[field.Name] = field;
                    }
                }
            }

            List<FieldInfo> result = new List<FieldInfo>(byName.Count);
            foreach (KeyValuePair<string, FieldInfo> kv in byName)
            {
                result.Add(kv.Value);
            }

            return result;
        }
    }

    /// <summary>
    /// Reflection fallback for fields that cannot be compiled into fast bridges.
    /// </summary>
    internal static class TypeFieldRegistrationFallback
    {
        internal static void PushStaticField(IntPtr luaState, FieldInfo field)
        {
            object value = field.GetValue(null);
            TypeMethodRegistration.PushReturnDefaultPublic(luaState, field.FieldType, value);
        }

        internal static void SetStaticField(IntPtr luaState, FieldInfo field)
        {
            AssignStaticField(luaState, field);
        }

        internal static int AssignStaticField(IntPtr luaState, FieldInfo field)
        {
            if (field.IsLiteral)
            {
                return LuaDllExtension.error(luaState, $"zlua: cannot assign to const field {field.DeclaringType.Name}.{field.Name}");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadValuePublic(luaState, 3, field.FieldType);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            field.SetValue(null, value);
            return 0;
        }

        internal static void PushInstanceField(IntPtr luaState, FieldInfo field)
        {
            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, 1, out object target))
            {
                LuaCallbackBoundary.Throw("zlua: invalid userdata for member access");
            }

            object value = field.GetValue(target);
            TypeMethodRegistration.PushReturnDefaultPublic(luaState, field.FieldType, value);
        }

        internal static void SetInstanceField(IntPtr luaState, FieldInfo field)
        {
            AssignInstanceField(luaState, field);
        }

        internal static int AssignInstanceField(IntPtr luaState, FieldInfo field)
        {
            if (field.IsInitOnly || field.IsLiteral)
            {
                return LuaDllExtension.error(luaState, $"zlua: cannot assign to read-only field {field.DeclaringType.Name}.{field.Name}");
            }

            if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, 1, out object target))
            {
                return LuaDllExtension.error(luaState, "zlua: invalid userdata for field assignment");
            }

            object value;
            try
            {
                value = TypeMethodRegistration.ReadValuePublic(luaState, 3, field.FieldType);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }

            field.SetValue(target, value);
            return 0;
        }
    }
}
