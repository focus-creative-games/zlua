using System;
using System.Collections.Generic;
using System.Reflection;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Mt
{
    /// <summary>
    /// Type table intern + dispatch to Reference / ValueType / Array creators.
    /// </summary>
    internal static class TypeRegistry
    {
        private static readonly Dictionary<Type, int> s_internedTypeTableRefs = new Dictionary<Type, int>();
        private static readonly Dictionary<string, Type> s_corlibTypeCache =
            new Dictionary<string, Type>(StringComparer.Ordinal);

        internal static void PushInternedTypeTable(IntPtr L, Type type)
        {
            if (type == null)
            {
                LuaCallbackBoundary.Throw("zlua: type is null");
            }

            if (s_internedTypeTableRefs.TryGetValue(type, out int existing))
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, existing);
                return;
            }

            PushTypeTable(L, type);
            LuaDll.lua_pushvalue(L, -1);
            int tableRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            s_internedTypeTableRefs[type] = tableRef;
        }

        internal static Type GetTypeFromTypeTable(IntPtr L, int index)
        {
            if (!LuaDll.lua_istable(L, index))
            {
                LuaCallbackBoundary.Throw("zlua: type table expected");
            }

            LuaDll.lua_getfield(L, index, LuaConsts.Klass);
            Type type = TypeHandleStore.ReadLightUserData(L, -1);
            LuaDll.lua_pop(L, 1);
            return type;
        }

        internal static Type ResolveTypeArg(IntPtr L, int index)
        {
            LuaDataType t = LuaDll.lua_type(L, index);
            if (t == LuaDataType.String)
            {
                string typeName = LuaDllExtension.tostring(L, index);
                return ResolveCorlibType(typeName);
            }

            if (t == LuaDataType.Table)
            {
                return GetTypeFromTypeTable(L, index);
            }

            return null;
        }

        internal static Type ResolveCorlibType(string typeName)
        {
            if (string.IsNullOrEmpty(typeName))
            {
                return null;
            }

            lock (s_corlibTypeCache)
            {
                if (s_corlibTypeCache.TryGetValue(typeName, out Type cached))
                {
                    return cached;
                }
            }

            // Do NOT use Type.GetType(string): StackCrawlMark SIGSEGVs under nested
            // lua_pcall on Unity Mono (native Lua frames on the managed stack).
            Type type = FindTypeWithoutStackCrawl(typeName);
            if (type != null)
            {
                lock (s_corlibTypeCache)
                {
                    s_corlibTypeCache[typeName] = type;
                }
            }

            return type;
        }

        private static Type FindTypeWithoutStackCrawl(string typeName)
        {
            Type type = typeof(object).Assembly.GetType(typeName, throwOnError: false);
            if (type != null)
            {
                return type;
            }

            Assembly[] assemblies;
            try
            {
                assemblies = AppDomain.CurrentDomain.GetAssemblies();
            }
            catch
            {
                return null;
            }

            for (int i = 0; i < assemblies.Length; i++)
            {
                Assembly asm = assemblies[i];
                if (asm == null)
                {
                    continue;
                }

                try
                {
                    type = asm.GetType(typeName, throwOnError: false);
                }
                catch
                {
                    continue;
                }

                if (type != null)
                {
                    return type;
                }
            }

            return null;
        }

        internal static string GetLuaFullName(Type type)
        {
            if (type == null)
            {
                return null;
            }

            var sb = new System.Text.StringBuilder(64);
            AppendLuaFullName(sb, type);
            return sb.ToString();
        }

        /// <summary>
        /// Il2Cpp-aligned type display name: nested uses '+', closed generics use
        /// <c>Name`n[Arg,...]</c> without assembly qualification.
        /// </summary>
        private static void AppendLuaFullName(System.Text.StringBuilder sb, Type type)
        {
            if (type.IsByRef)
            {
                AppendLuaFullName(sb, type.GetElementType());
                sb.Append('&');
                return;
            }

            if (type.IsPointer)
            {
                AppendLuaFullName(sb, type.GetElementType());
                sb.Append('*');
                return;
            }

            if (type.IsArray)
            {
                AppendLuaFullName(sb, type.GetElementType());
                int rank = type.GetArrayRank();
                if (type.IsSZArray || (rank == 1 && type.Name.EndsWith("[]", StringComparison.Ordinal)))
                {
                    sb.Append("[]");
                }
                else
                {
                    sb.Append('[');
                    for (int i = 1; i < rank; i++)
                    {
                        sb.Append(',');
                    }

                    sb.Append(']');
                }

                return;
            }

            Type declaring = type.DeclaringType;
            if (declaring != null)
            {
                AppendLuaFullName(sb, declaring);
                sb.Append('+');
            }
            else if (!string.IsNullOrEmpty(type.Namespace))
            {
                sb.Append(type.Namespace);
                sb.Append('.');
            }

            if (type.IsGenericType)
            {
                string name = type.Name;
                sb.Append(name);
                if (!type.IsGenericTypeDefinition)
                {
                    Type[] args = type.GetGenericArguments();
                    sb.Append('[');
                    for (int i = 0; i < args.Length; i++)
                    {
                        if (i > 0)
                        {
                            sb.Append(',');
                        }

                        AppendLuaFullName(sb, args[i]);
                    }

                    sb.Append(']');
                }
            }
            else
            {
                sb.Append(type.Name);
            }
        }

        /// <summary>
        /// Push a newly constructed instance with declared-type facade.
        /// </summary>
        internal static void PushConstructorInstance(IntPtr L, object instance, Type declaredType)
        {
            if (instance == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            if (declaredType == null)
            {
                declaredType = instance.GetType();
            }

            if (StructMarshal.IsStructType(declaredType))
            {
                StructMarshal.PushValue(L, instance, declaredType);
                return;
            }

            ObjectMarshal.Push(L, instance, declaredType);
        }

        private static void PushTypeTable(IntPtr L, Type type)
        {
            TypeMemberLuaIndexer.EnsureLoaded(L);

            if (type.IsArray)
            {
                TypeRegistryArray.CreateTypeTable(L, type);
                return;
            }

            if (type.IsEnum)
            {
                TypeRegistryValueType.CreateEnumTypeTable(L, type);
                return;
            }

            Type nullableUnderlying = Nullable.GetUnderlyingType(type);
            if (nullableUnderlying != null)
            {
                TypeRegistryValueType.CreateNullableTypeTable(L, type);
                return;
            }

            if (type.IsValueType)
            {
                TypeRegistryValueType.CreateTypeTable(L, type);
                return;
            }

            // class / interface / delegate / string / object
            TypeRegistryReference.CreateTypeTable(L, type);
        }
    }
}
