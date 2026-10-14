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
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using ZLua.Emit;
using ZLua.Utils;

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
    /// Registry refs to the three indexer tables (filled when a type table is created for the current lua_State).
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

        /// <summary>
        /// Drop all type bindings (member maps, three-table refs).
        /// Aligns with Il2Cpp <c>MetaBinding::ShutdownState</c>; call on Reset / <c>lua_close</c>.
        /// </summary>
        internal static void Shutdown()
        {
            s_bindings.Clear();
        }

        /// <summary>
        /// Find an open generic method for <c>zlua.inflate</c> by short name or full-signature key.
        /// Searches static + ByObj instance maps (inflate returns ByObj-convention instance closures).
        /// </summary>
        internal static bool TryFindOpenGenericMethod(
            Type ownerType,
            string nameOrSignature,
            int genericArity,
            out MethodInfo openMethod,
            out bool isStatic,
            out bool isByVal)
        {
            openMethod = null;
            isStatic = false;
            isByVal = false;
            if (ownerType == null || string.IsNullOrEmpty(nameOrSignature) || genericArity < 1)
            {
                return false;
            }

            TypeBinding binding = EnsureBinding(ownerType);
            bool isSignature = nameOrSignature.IndexOf('(') >= 0;
            MethodInfo match = null;
            bool matchStatic = false;
            bool matchByVal = false;
            int matchCount = 0;

            CollectOpenGenericMatches(
                binding.StaticMap,
                nameOrSignature,
                isSignature,
                genericArity,
                isStaticCandidate: true,
                isByValCandidate: false,
                ref match,
                ref matchStatic,
                ref matchByVal,
                ref matchCount);

            CollectOpenGenericMatches(
                binding.ByObjInstanceMap,
                nameOrSignature,
                isSignature,
                genericArity,
                isStaticCandidate: false,
                isByValCandidate: false,
                ref match,
                ref matchStatic,
                ref matchByVal,
                ref matchCount);

            if (matchCount != 1)
            {
                return false;
            }

            openMethod = match;
            isStatic = matchStatic;
            isByVal = matchByVal;
            return true;
        }

        private static void CollectOpenGenericMatches(
            Dictionary<string, MetaInfo> map,
            string nameOrSignature,
            bool isSignature,
            int genericArity,
            bool isStaticCandidate,
            bool isByValCandidate,
            ref MethodInfo match,
            ref bool matchStatic,
            ref bool matchByVal,
            ref int matchCount)
        {
            if (map == null || map.Count == 0)
            {
                return;
            }

            foreach (KeyValuePair<string, MetaInfo> kv in map)
            {
                MetaInfo info = kv.Value;
                if (info == null || info.Kind != MetaKind.Method)
                {
                    continue;
                }

                if (!isSignature
                    && !string.Equals(info.Name, nameOrSignature, StringComparison.Ordinal))
                {
                    // Short-name lookup uses Lua final name (default or [LuaAlias]).
                    continue;
                }

                List<MethodInfo> methods = info.MethodOverloads;
                if (methods == null || methods.Count == 0)
                {
                    if (info.Method == null)
                    {
                        continue;
                    }

                    methods = new List<MethodInfo> { info.Method };
                }

                for (int i = 0; i < methods.Count; i++)
                {
                    MethodInfo method = methods[i];
                    if (method == null || !IsOpenGenericMethod(method))
                    {
                        continue;
                    }

                    if (method.GetGenericArguments().Length != genericArity)
                    {
                        continue;
                    }

                    if (isSignature)
                    {
                        if (!string.Equals(
                                BuildOpenMethodSignatureKey(method),
                                nameOrSignature,
                                StringComparison.Ordinal))
                        {
                            continue;
                        }
                    }

                    matchCount++;
                    if (matchCount == 1)
                    {
                        match = method;
                        matchStatic = isStaticCandidate;
                        matchByVal = isByValCandidate;
                    }
                }
            }
        }

        private static bool IsOpenGenericMethod(MethodInfo method)
        {
            return method.IsGenericMethodDefinition || method.ContainsGenericParameters;
        }

        /// <summary>
        /// Full-signature key: <c>Name(ParamFullNames…)</c> (CLR name, not Lua alias).
        /// Extension methods omit the <c>this</c> parameter. Excludes ctors (callers must not pass them).
        /// </summary>
        internal static string BuildMethodSignatureKey(MethodInfo method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            int start = ExtensionMethodUtil.IsExtensionMethod(method) ? 1 : 0;
            var sb = new StringBuilder(method.Name.Length + 16 + (parameters.Length - start) * 24);
            sb.Append(method.Name);
            sb.Append('(');
            for (int i = start; i < parameters.Length; i++)
            {
                if (i > start)
                {
                    sb.Append(',');
                }

                sb.Append(TypeRegistry.GetLuaFullName(parameters[i].ParameterType));
            }

            sb.Append(')');
            return sb.ToString();
        }

        /// <summary>Alias for open-generic inflate lookup; same format as <see cref="BuildMethodSignatureKey"/>.</summary>
        internal static string BuildOpenMethodSignatureKey(MethodInfo method)
        {
            return BuildMethodSignatureKey(method);
        }

        /// <summary>
        /// Find first public method whose §3.7 signature key matches (type + bases).
        /// Never matches instance or static constructors.
        /// </summary>
        internal static bool TryFindMethodBySignatureKey(
            Type ownerType,
            string signatureKey,
            bool isStatic,
            out MethodInfo method)
        {
            method = null;
            if (ownerType == null || string.IsNullOrEmpty(signatureKey) || signatureKey.IndexOf('(') < 0)
            {
                return false;
            }

            BindingFlags flags = BindingFlags.Public | BindingFlags.DeclaredOnly
                | (isStatic ? BindingFlags.Static : BindingFlags.Instance);

            Type walk = ownerType;
            while (walk != null
                   && walk != typeof(object)
                   && walk != typeof(ValueType)
                   && walk != typeof(Enum))
            {
                MethodInfo[] methods = walk.GetMethods(flags);
                for (int i = 0; i < methods.Length; i++)
                {
                    MethodInfo candidate = methods[i];
                    if (ShouldSkipSignatureResolve(candidate))
                    {
                        continue;
                    }

                    if (string.Equals(
                            BuildMethodSignatureKey(candidate),
                            signatureKey,
                            StringComparison.Ordinal))
                    {
                        method = candidate;
                        return true;
                    }
                }

                walk = walk.BaseType;
            }

            return false;
        }

        /// <summary>
        /// Resolve full-signature key, emit direct/GTAP closure into method table + map, push closure.
        /// Returns 1 if pushed; 0 if not found (caller pushes nil / continues miss).
        /// </summary>
        internal static int TryResolveSignatureMethodAndPush(
            IntPtr L,
            Type ownerType,
            string signatureKey,
            bool isStatic,
            bool isByVal)
        {
            if (!TryFindMethodBySignatureKey(ownerType, signatureKey, isStatic, out MethodInfo method))
            {
                return 0;
            }

            TypeBinding binding = EnsureBinding(ownerType);
            MemberTableSet tables = isStatic
                ? binding.StaticTables
                : (isByVal ? binding.ByValInstanceTables : binding.ByObjInstanceTables);
            Dictionary<string, MetaInfo> map = isStatic
                ? binding.StaticMap
                : (isByVal ? binding.ByValInstanceMap : binding.ByObjInstanceMap);

            if (tables == null || !tables.IsValid)
            {
                return 0;
            }

            if (!map.TryGetValue(signatureKey, out MetaInfo info) || info == null || info.Kind != MetaKind.Method)
            {
                info = new MetaInfo
                {
                    Kind = MetaKind.Method,
                    Name = signatureKey,
                    IsStatic = isStatic,
                    Method = method,
                };
                map[signatureKey] = info;
                MethodEmitter.Write(L, tables.MethodTableRef, info, ownerType, isStatic, isByVal);
            }

            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, tables.MethodTableRef);
            LuaDll.lua_getfield(L, -1, signatureKey);
            LuaDll.lua_remove(L, -2);
            if (LuaDll.lua_isnil(L, -1))
            {
                // Soft-skip emit left no closure; drop stale map entry.
                map.Remove(signatureKey);
                LuaDll.lua_pop(L, 1);
                return 0;
            }

            return 1;
        }

        private static bool ShouldSkipSignatureResolve(MethodInfo method)
        {
            if (method == null)
            {
                return true;
            }

            // Instance ctor / static ctor (.cctor) — never exposed via full-signature keys.
            if (method.IsConstructor
                || string.Equals(method.Name, ".ctor", StringComparison.Ordinal)
                || string.Equals(method.Name, ".cctor", StringComparison.Ordinal))
            {
                return true;
            }

            if (method.IsSpecialName
                && (method.Name.StartsWith("get_", StringComparison.Ordinal)
                    || method.Name.StartsWith("set_", StringComparison.Ordinal)))
            {
                return true;
            }

            if (method.DeclaringType == typeof(object) && method.Name == "Finalize")
            {
                return true;
            }

            return false;
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
