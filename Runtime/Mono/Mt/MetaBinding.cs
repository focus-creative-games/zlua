using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua.Mt
{
    internal enum MetaKind : byte
    {
        Method = 0,
        Field = 1,
        Property = 2,
    }

    /// <summary>
    /// Bind-time member metadata. Lua closures are filled in Phase 3 Emit.
    /// </summary>
    internal sealed class MetaInfo
    {
        public MetaKind Kind;
        public string Name;
        public bool IsStatic;

        public MethodInfo Method;
        public FieldInfo Field;
        public PropertyInfo Property;

        /// <summary>Overload group (same name); Phase 3 resolves.</summary>
        public List<MethodInfo> MethodOverloads;
    }

    /// <summary>
    /// Permanent registry refs to the three indexer tables (filled in Phase 3).
    /// </summary>
    internal sealed class MemberTableSet
    {
        public int MethodTableRef = LuaConsts.LuaNoRef;
        public int FieldGetterTableRef = LuaConsts.LuaNoRef;
        public int FieldSetterTableRef = LuaConsts.LuaNoRef;

        public bool IsValid =>
            MethodTableRef != LuaConsts.LuaNoRef
            && FieldGetterTableRef != LuaConsts.LuaNoRef
            && FieldSetterTableRef != LuaConsts.LuaNoRef;
    }

    internal sealed class TypeBinding
    {
        public Type Type;
        public MemberTableSet StaticTables;
        public MemberTableSet ByObjInstanceTables;
        public MemberTableSet ByValInstanceTables;

        public readonly Dictionary<string, MetaInfo> StaticMap =
            new Dictionary<string, MetaInfo>(StringComparer.Ordinal);
        public readonly Dictionary<string, MetaInfo> ByObjInstanceMap =
            new Dictionary<string, MetaInfo>(StringComparer.Ordinal);
        public readonly Dictionary<string, MetaInfo> ByValInstanceMap =
            new Dictionary<string, MetaInfo>(StringComparer.Ordinal);

        public readonly List<ConstructorInfo> Constructors = new List<ConstructorInfo>();
    }

    /// <summary>
    /// Scans public members into <see cref="TypeBinding"/> maps. No Event special-case (D5).
    /// Does not Emit Lua bridges (Phase 3).
    /// </summary>
    internal static class MetaBinding
    {
        private static readonly Dictionary<Type, TypeBinding> s_bindings = new Dictionary<Type, TypeBinding>();

        private const BindingFlags PublicDeclared =
        BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly;

        internal static TypeBinding EnsureBinding(Type type)
        {
            if (type == null)
            {
                throw new ArgumentNullException(nameof(type));
            }

            if (s_bindings.TryGetValue(type, out TypeBinding existing))
            {
                return existing;
            }

            TypeBinding binding = new TypeBinding { Type = type };
            BuildBinding(binding);
            s_bindings[type] = binding;
            return binding;
        }

        internal static bool TryGetBinding(Type type, out TypeBinding binding)
        {
            return s_bindings.TryGetValue(type, out binding);
        }

        private static void BuildBinding(TypeBinding binding)
        {
            Type type = binding.Type;
            if (type.IsGenericTypeDefinition)
            {
                return;
            }

            CollectConstructors(binding, type);
            CollectMethods(binding, type);
            CollectExtensionMethods(binding, type);
            CollectFieldsAndProperties(binding, type);
        }

        private static void CollectConstructors(TypeBinding binding, Type type)
        {
            ConstructorInfo[] ctors = type.GetConstructors(BindingFlags.Public | BindingFlags.Instance);
            for (int i = 0; i < ctors.Length; i++)
            {
                binding.Constructors.Add(ctors[i]);
            }
        }

        private static void CollectMethods(TypeBinding binding, Type type)
        {
            // Include inherited public methods (virtual dispatch view uses declared-type facade later).
            MethodInfo[] methods = type.GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static);
            for (int i = 0; i < methods.Length; i++)
            {
                MethodInfo method = methods[i];
                if (method.IsConstructor)
                {
                    continue;
                }

                // Property accessors are registered via PropertyInfo; keep add_/remove_ as methods (D5).
                if (method.IsSpecialName
                    && (method.Name.StartsWith("get_", StringComparison.Ordinal)
                        || method.Name.StartsWith("set_", StringComparison.Ordinal)))
                {
                    continue;
                }

                // Skip object finalizer noise
                if (method.DeclaringType == typeof(object) && method.Name == "Finalize")
                {
                    continue;
                }

                // Spec: [LuaAlias] / XML alias replaces MethodInfo.Name (do not also keep the default name).
                string finalName = GetLuaFinalName(method);
                Dictionary<string, MetaInfo> map = method.IsStatic
                    ? binding.StaticMap
                    : binding.ByObjInstanceMap;

                if (map.TryGetValue(finalName, out MetaInfo existing))
                {
                    if (existing.Kind != MetaKind.Method)
                    {
                        continue; // methodTable wins only if first; keep first kind
                    }

                    if (existing.MethodOverloads == null)
                    {
                        existing.MethodOverloads = new List<MethodInfo> { existing.Method };
                    }

                    existing.MethodOverloads.Add(method);
                    continue;
                }

                map[finalName] = new MetaInfo
                {
                    Kind = MetaKind.Method,
                    Name = finalName,
                    IsStatic = method.IsStatic,
                    Method = method,
                };
            }

            // Value types: ByVal instance map mirrors ByObj method names (Emit differs in Phase 3).
            if (NeedsByValInstanceMap(type))
            {
                foreach (KeyValuePair<string, MetaInfo> kv in binding.ByObjInstanceMap)
                {
                    if (kv.Value.Kind == MetaKind.Method && !kv.Value.IsStatic)
                    {
                        binding.ByValInstanceMap[kv.Key] = kv.Value;
                    }
                }
            }
        }

        /// <summary>
        /// Collect C# extension methods into IMT (spec 13-EXTENSION-METHODS).
        /// Attribute ∪ XML on T and base types; filter ExtensionAttribute + this assignable from T.
        /// </summary>
        private static void CollectExtensionMethods(TypeBinding binding, Type type)
        {
            List<Type> extensionClasses = ResolveExtensionClasses(type);
            if (extensionClasses.Count == 0)
            {
                return;
            }

            const BindingFlags extFlags =
                BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly;

            for (int c = 0; c < extensionClasses.Count; c++)
            {
                Type extensionClass = extensionClasses[c];
                MethodInfo[] methods = extensionClass.GetMethods(extFlags);
                for (int i = 0; i < methods.Length; i++)
                {
                    MethodInfo method = methods[i];
                    if (!ExtensionMethodUtil.IsExtensionMethod(method))
                    {
                        continue;
                    }

                    if (method.ContainsGenericParameters || method.IsGenericMethodDefinition)
                    {
                        continue;
                    }

                    ParameterInfo[] parameters = method.GetParameters();
                    if (parameters.Length < 1)
                    {
                        continue;
                    }

                    Type thisType = parameters[0].ParameterType;
                    if (thisType.IsByRef)
                    {
                        thisType = thisType.GetElementType();
                    }

                    if (thisType == null || !thisType.IsAssignableFrom(type))
                    {
                        continue;
                    }

                    RegisterInstanceMethod(binding, type, method);
                }
            }
        }

        private static List<Type> ResolveExtensionClasses(Type type)
        {
            var result = new List<Type>();
            var seen = new HashSet<Type>();

            Type walk = type;
            while (walk != null && walk != typeof(object) && walk != typeof(ValueType) && walk != typeof(Enum))
            {
                object[] attrs = walk.GetCustomAttributes(typeof(LuaExtensionAttribute), inherit: false);
                for (int i = 0; i < attrs.Length; i++)
                {
                    var attr = (LuaExtensionAttribute)attrs[i];
                    Type[] extTypes = attr.ExtensionTypes;
                    if (extTypes == null)
                    {
                        continue;
                    }

                    for (int j = 0; j < extTypes.Length; j++)
                    {
                        Type ext = extTypes[j];
                        if (ext == null)
                        {
                            throw new LuaExtensionConfigurationException(
                                "[ZLua] [LuaExtension] on " + walk.FullName
                                + " contains a null extension type.");
                        }

                        if (seen.Add(ext))
                        {
                            result.Add(ext);
                        }
                    }
                }

                if (LuaExtensionXmlRegistry.TryGetExtensionTypes(walk, out Type[] xmlTypes))
                {
                    for (int i = 0; i < xmlTypes.Length; i++)
                    {
                        Type ext = xmlTypes[i];
                        if (ext != null && seen.Add(ext))
                        {
                            result.Add(ext);
                        }
                    }
                }

                walk = walk.BaseType;
            }

            return result;
        }

        private static void RegisterInstanceMethod(TypeBinding binding, Type ownerType, MethodInfo method)
        {
            string finalName = GetLuaFinalName(method);
            Dictionary<string, MetaInfo> map = binding.ByObjInstanceMap;

            if (map.TryGetValue(finalName, out MetaInfo existing))
            {
                if (existing.Kind != MetaKind.Method)
                {
                    return;
                }

                if (existing.MethodOverloads == null)
                {
                    existing.MethodOverloads = new List<MethodInfo> { existing.Method };
                }

                existing.MethodOverloads.Add(method);
                return;
            }

            MetaInfo info = new MetaInfo
            {
                Kind = MetaKind.Method,
                Name = finalName,
                IsStatic = false,
                Method = method,
            };
            map[finalName] = info;
            if (NeedsByValInstanceMap(ownerType))
            {
                binding.ByValInstanceMap[finalName] = info;
            }
        }

        private static void CollectFieldsAndProperties(TypeBinding binding, Type type)
        {
            // Flatten inheritance: walk from most-derived to base; first wins.
            Type walk = type;
            while (walk != null && walk != typeof(object) && walk != typeof(ValueType) && walk != typeof(Enum))
            {
                FieldInfo[] fields = walk.GetFields(PublicDeclared);
                for (int i = 0; i < fields.Length; i++)
                {
                    FieldInfo field = fields[i];
                    Dictionary<string, MetaInfo> map = field.IsStatic
                        ? binding.StaticMap
                        : binding.ByObjInstanceMap;
                    if (map.ContainsKey(field.Name))
                    {
                        continue;
                    }

                    MetaInfo info = new MetaInfo
                    {
                        Kind = MetaKind.Field,
                        Name = field.Name,
                        IsStatic = field.IsStatic,
                        Field = field,
                    };
                    map[field.Name] = info;
                    if (!field.IsStatic && NeedsByValInstanceMap(type))
                    {
                        binding.ByValInstanceMap[field.Name] = info;
                    }
                }

                PropertyInfo[] props = walk.GetProperties(PublicDeclared);
                for (int i = 0; i < props.Length; i++)
                {
                    PropertyInfo prop = props[i];
                    ParameterInfo[] indexParams = prop.GetIndexParameters();
                    // Zero-param → field getter/setter tables; indexed → methodTable (Phase 3).
                    bool isIndexer = indexParams != null && indexParams.Length > 0;
                    MethodInfo getter = prop.GetGetMethod(nonPublic: false);
                    MethodInfo setter = prop.GetSetMethod(nonPublic: false);
                    bool isStatic = (getter != null && getter.IsStatic) || (setter != null && setter.IsStatic);

                    Dictionary<string, MetaInfo> map = isStatic
                        ? binding.StaticMap
                        : binding.ByObjInstanceMap;

                    // Il2Cpp: indexers stay as get_Item / set_Item methods (not prop.Name "Item").
                    if (isIndexer)
                    {
                        RegisterIndexerAccessor(map, binding, type, getter, isStatic);
                        RegisterIndexerAccessor(map, binding, type, setter, isStatic);
                        continue;
                    }

                    if (map.ContainsKey(prop.Name))
                    {
                        continue;
                    }

                    MetaInfo info = new MetaInfo
                    {
                        Kind = MetaKind.Property,
                        Name = prop.Name,
                        IsStatic = isStatic,
                        Property = prop,
                    };
                    map[prop.Name] = info;
                    if (!isStatic && NeedsByValInstanceMap(type))
                    {
                        binding.ByValInstanceMap[prop.Name] = info;
                    }
                }

                walk = walk.BaseType;
            }
        }

        private static void RegisterIndexerAccessor(
            Dictionary<string, MetaInfo> map,
            TypeBinding binding,
            Type ownerType,
            MethodInfo accessor,
            bool isStatic)
        {
            if (accessor == null || map.ContainsKey(accessor.Name))
            {
                return;
            }

            MetaInfo info = new MetaInfo
            {
                Kind = MetaKind.Method,
                Name = accessor.Name,
                IsStatic = isStatic,
                Method = accessor,
            };
            map[accessor.Name] = info;
            if (!isStatic && NeedsByValInstanceMap(ownerType))
            {
                binding.ByValInstanceMap[accessor.Name] = info;
            }
        }

        /// <summary>
        /// Final Lua key: Attribute alias &gt; XML alias &gt; MethodInfo.Name.
        /// When an alias is present, the default C# name is not registered.
        /// </summary>
        private static string GetLuaFinalName(MethodInfo method)
        {
            LuaAliasAttribute alias = method.GetCustomAttribute<LuaAliasAttribute>(inherit: false);
            if (alias != null && !string.IsNullOrEmpty(alias.Alias))
            {
                return alias.Alias;
            }

            if (LuaAliasXmlRegistry.TryGetAlias(method, out string xmlAlias)
                && !string.IsNullOrEmpty(xmlAlias))
            {
                return xmlAlias;
            }

            return method.Name;
        }

        private static bool NeedsByValInstanceMap(Type type)
        {
            return type.IsValueType && !type.IsEnum && !type.IsPrimitive;
        }
    }
}
