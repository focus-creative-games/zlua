using System;
using System.Collections.Generic;
using System.Reflection;
using ZLua.Emit;
using ZLua.Marshaling;
using ZLua.Utils;

namespace ZLua.Mt
{
    /// <summary>
    /// Shared type-table helpers: common fields, empty three-table indexer attach, SMT/IMT.
    /// </summary>
    internal static class TypeRegistryCommon
    {
        private static readonly List<LuaCSFunction> s_pins = new List<LuaCSFunction>();
        private static readonly LuaCSFunction s_typeTableToString = TypeTableToString;
        private static readonly LuaCSFunction s_ctorNotReady = ConstructorNotReady;
        private static readonly LuaCSFunction s_structDefault = InvokeStructDefault;
        private static readonly LuaCSFunction s_delegateInstanceCall = DelegateInstanceCall;
        private static bool s_pinned;

        internal static MemberTableSet CreateEmptyMemberTableSet(IntPtr L)
        {
            MemberTableSet set = new MemberTableSet();
            LuaDll.lua_createtable(L, 0, 0);
            set.MethodTableRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            LuaDll.lua_createtable(L, 0, 0);
            set.FieldGetterTableRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            LuaDll.lua_createtable(L, 0, 0);
            set.FieldSetterTableRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
            return set;
        }

        internal static void PushMemberTableSet(IntPtr L, MemberTableSet set)
        {
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, set.MethodTableRef);
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, set.FieldGetterTableRef);
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, set.FieldSetterTableRef);
        }

        /// <summary>
        /// Registers a native (non-Emit) instance method into the ByObj method table + map (Il2Cpp array get/set).
        /// </summary>
        internal static void RegisterNativeInstanceMethod(IntPtr L, TypeBinding binding, string name, LuaCSFunction fn)
        {
            if (binding == null || string.IsNullOrEmpty(name) || fn == null)
            {
                throw new ArgumentException("RegisterNativeInstanceMethod requires binding, name, and function");
            }

            if (binding.ByObjInstanceMap.ContainsKey(name))
            {
                LuaCallbackBoundary.Throw($"zlua: duplicate instance method: {name}");
            }

            if (binding.ByObjInstanceTables == null || !binding.ByObjInstanceTables.IsValid)
            {
                LuaCallbackBoundary.Throw($"zlua: instance method table not bound for {binding.Type?.FullName}");
            }

            ClosurePin.WriteToTable(L, binding.ByObjInstanceTables.MethodTableRef, name, fn);
            binding.ByObjInstanceMap[name] = new MetaInfo
            {
                Kind = MetaKind.Method,
                Name = name,
                IsStatic = false,
            };
        }

        internal static void WriteCommonTypeFields(IntPtr L, Type type, int typeTableIndex)
        {
            LuaDll.lua_pushstring(L, TypeRegistry.GetLuaFullName(type));
            LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.FullName);
            TypeHandleStore.PushLightUserData(L, type);
            LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.Klass);
        }

        internal static void RegisterStaticLiteralFields(IntPtr L, Type type, int typeTableIndex)
        {
            FieldInfo[] fields = type.GetFields(BindingFlags.Public | BindingFlags.Static);
            for (int i = 0; i < fields.Length; i++)
            {
                FieldInfo field = fields[i];
                if (!field.IsLiteral && !field.IsInitOnly)
                {
                    continue;
                }

                // Enum constants and const primitives → write onto type table when possible.
                object value;
                try
                {
                    value = field.GetValue(null);
                }
                catch
                {
                    continue;
                }

                Type fieldType = field.FieldType;
                if (fieldType.IsEnum)
                {
                    object underlying = Convert.ChangeType(value, Enum.GetUnderlyingType(fieldType));
                    PushIntegerLike(L, underlying);
                    LuaDll.lua_setfield(L, typeTableIndex, field.Name);
                    continue;
                }

                if (fieldType.IsPrimitive || fieldType == typeof(string))
                {
                    TypedMarshal.PushObject(L, value, fieldType);
                    LuaDll.lua_setfield(L, typeTableIndex, field.Name);
                }
            }
        }

        internal static void AttachReferenceInstanceMetatable(IntPtr L, Type type, int typeTableIndex, TypeBinding binding)
        {
            AttachByObjInstanceMetatableCore(L, type, typeTableIndex, binding, ObjectRegistry.GetOnReleaseFunctionPointer());
        }

        internal static void AttachByObjInstanceMetatable(IntPtr L, Type type, int typeTableIndex, TypeBinding binding)
        {
            AttachByObjInstanceMetatableCore(L, type, typeTableIndex, binding, ObjectRegistry.GetOnReleaseFunctionPointer());
        }

        internal static void AttachByValInstanceMetatable(IntPtr L, Type type, int typeTableIndex, TypeBinding binding)
        {
            EnsurePinned();
            TypeMemberLuaIndexer.EnsureLoaded(L);

            LuaDll.lua_createtable(L, 0, 8);
            int mtIndex = LuaDll.lua_gettop(L);

            LuaDll.lua_pushvalue(L, typeTableIndex);
            LuaDll.lua_setfield(L, mtIndex, LuaConsts.Type);
            LuaDll.lua_pushstring(L, LuaConsts.UdKindByVal);
            LuaDll.lua_setfield(L, mtIndex, LuaConsts.UdKind);

            if (!StructMarshal.IsBlittable(type))
            {
                LuaDll.lua_pushcfunction(L, StructRegistry.GetOnReleaseFunctionPointer());
                LuaDll.lua_setfield(L, mtIndex, LuaConsts.MetaGc);
            }

            binding.ByValInstanceTables = CreateEmptyMemberTableSet(L);
            PushMemberTableSet(L, binding.ByValInstanceTables);
            int setterIdx = LuaDll.lua_gettop(L);
            int getterIdx = setterIdx - 1;
            int methodIdx = setterIdx - 2;
            TypeMemberLuaIndexer.BindInstanceMetatable(L, mtIndex, methodIdx, getterIdx, setterIdx);
            LuaDll.lua_pop(L, 3);

            LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.ByValInstanceMt);
        }

        internal static void AttachStaticTypeMetatable(
            IntPtr L,
            Type type,
            int typeTableIndex,
            TypeBinding binding,
            bool enableConstructorCall,
            bool enableStructDefault)
        {
            EnsurePinned();
            TypeMemberLuaIndexer.EnsureLoaded(L);

            LuaDll.lua_createtable(L, 0, 8);
            int smtIndex = LuaDll.lua_gettop(L);

            if (enableConstructorCall)
            {
                TypeHandleStore.PushLightUserData(L, type);
                LuaDll.lua_pushcclosure(L, global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_ctorNotReady), 1);
                LuaDll.lua_setfield(L, smtIndex, LuaConsts.MetaCall);
            }

            if (enableStructDefault)
            {
                TypeHandleStore.PushLightUserData(L, type);
                LuaDll.lua_pushcclosure(L, global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_structDefault), 1);
                LuaDll.lua_setfield(L, smtIndex, LuaConsts.Default);
            }

            LuaDll.lua_pushcfunction(L, global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_typeTableToString));
            LuaDll.lua_setfield(L, smtIndex, LuaConsts.MetaToString);

            binding.StaticTables = CreateEmptyMemberTableSet(L);
            PushMemberTableSet(L, binding.StaticTables);
            int setterIdx = LuaDll.lua_gettop(L);
            int getterIdx = setterIdx - 1;
            int methodIdx = setterIdx - 2;
            TypeMemberLuaIndexer.BindStaticMetatable(L, smtIndex, methodIdx, getterIdx, setterIdx);
            LuaDll.lua_pop(L, 3);

            LuaDll.lua_setmetatable(L, typeTableIndex);
        }

        private static void AttachByObjInstanceMetatableCore(
            IntPtr L,
            Type type,
            int typeTableIndex,
            TypeBinding binding,
            IntPtr gcFn)
        {
            EnsurePinned();
            TypeMemberLuaIndexer.EnsureLoaded(L);

            LuaDll.lua_createtable(L, 0, 8);
            int mtIndex = LuaDll.lua_gettop(L);

            LuaDll.lua_pushvalue(L, typeTableIndex);
            LuaDll.lua_setfield(L, mtIndex, LuaConsts.Type);
            LuaDll.lua_pushstring(L, LuaConsts.UdKindByObj);
            LuaDll.lua_setfield(L, mtIndex, LuaConsts.UdKind);

            if (gcFn != IntPtr.Zero)
            {
                LuaDll.lua_pushcfunction(L, gcFn);
                LuaDll.lua_setfield(L, mtIndex, LuaConsts.MetaGc);
            }

            binding.ByObjInstanceTables = CreateEmptyMemberTableSet(L);
            PushMemberTableSet(L, binding.ByObjInstanceTables);
            int setterIdx = LuaDll.lua_gettop(L);
            int getterIdx = setterIdx - 1;
            int methodIdx = setterIdx - 2;
            TypeMemberLuaIndexer.BindInstanceMetatable(L, mtIndex, methodIdx, getterIdx, setterIdx);
            LuaDll.lua_pop(L, 3);

            if (typeof(Delegate).IsAssignableFrom(type))
            {
                LuaDll.lua_pushcfunction(
                    L,
                    global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_delegateInstanceCall));
                LuaDll.lua_setfield(L, mtIndex, LuaConsts.MetaCall);
            }

            LuaDll.lua_setfield(L, typeTableIndex, LuaConsts.ByObjInstanceMt);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DelegateInstanceCall(IntPtr L)
        {
            try
            {
                object obj = ObjectRegistry.PopThis(L, 1);
                return DelegateInvokerCache.Invoke(obj as Delegate, L);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int TypeTableToString(IntPtr L)
        {
            try
            {
                // __tostring(typeTable) — self is argument 1
                LuaDll.lua_getfield(L, 1, LuaConsts.FullName);
                if (LuaDll.lua_type(L, -1) != LuaDataType.String)
                {
                    LuaDll.lua_pop(L, 1);
                    LuaDll.lua_pushstring(L, "zlua.type");
                }

                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ConstructorNotReady(IntPtr L)
        {
            Type type = TypeHandleStore.ReadLightUserData(L, LuaDll.lua_upvalueindex(1));
            string name = type != null ? TypeRegistry.GetLuaFullName(type) : "?";
            return LuaDllExtension.error(L, $"zlua: constructor for {name} not bound yet (Phase 3 Emit)");
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStructDefault(IntPtr L)
        {
            try
            {
                Type type = TypeHandleStore.ReadLightUserData(L, LuaDll.lua_upvalueindex(1));
                if (type == null)
                {
                    LuaCallbackBoundary.Throw("zlua: _default missing type");
                }

                if (LuaDll.lua_gettop(L) != 0)
                {
                    LuaCallbackBoundary.Throw($"zlua: {TypeRegistry.GetLuaFullName(type)}._default expects no arguments");
                }

                StructMarshal.PushZeroedValue(L, type);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(L, ex);
            }
        }

        private static void PushIntegerLike(IntPtr L, object underlying)
        {
            switch (underlying)
            {
                case byte b: LuaDll.lua_pushinteger(L, b); break;
                case sbyte sb: LuaDll.lua_pushinteger(L, sb); break;
                case short s: LuaDll.lua_pushinteger(L, s); break;
                case ushort us: LuaDll.lua_pushinteger(L, us); break;
                case int i: LuaDll.lua_pushinteger(L, i); break;
                case uint ui: LuaDll.lua_pushinteger(L, ui); break;
                case long l: LuaDll.lua_pushinteger(L, l); break;
                case ulong ul: LuaDll.lua_pushinteger(L, (long)ul); break;
                default: LuaDll.lua_pushinteger(L, Convert.ToInt64(underlying)); break;
            }
        }

        private static void EnsurePinned()
        {
            if (s_pinned)
            {
                return;
            }

            s_pins.Add(s_typeTableToString);
            s_pins.Add(s_ctorNotReady);
            s_pins.Add(s_structDefault);
            s_pins.Add(s_delegateInstanceCall);
            s_pinned = true;
        }
    }
}
