using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace ZLua
{
    public sealed class LuaManagerObject
    {
        private static readonly Dictionary<int, Assembly> Assemblies = new Dictionary<int, Assembly>();
        private static readonly Dictionary<int, Type> Types = new Dictionary<int, Type>();
        private static readonly Dictionary<string, int> AssemblyIds = new Dictionary<string, int>(StringComparer.Ordinal);
        private static readonly Dictionary<string, Assembly> AssemblyByLuaName = new Dictionary<string, Assembly>(StringComparer.Ordinal);
        private static readonly Dictionary<string, Type> TypeCache = new Dictionary<string, Type>(StringComparer.Ordinal);
        private static readonly Dictionary<Type, int> TypeTableRefs = new Dictionary<Type, int>();
        private static readonly Dictionary<int, int> StaticMethodTableRefsByTypeId = new Dictionary<int, int>();
        private static readonly Dictionary<int, int> InstanceMethodTableRefsByTypeId = new Dictionary<int, int>();
        private static readonly Assembly Mscorlib = typeof(object).Assembly;

        private static readonly List<LuaCSFunction> CallbackRefs = new List<LuaCSFunction>();
        private static readonly LuaCSFunction CSharpIndexCallback = ResolveAssemblyIndex;
        private static readonly LuaCSFunction AssemblyTypeIndexCallback = ResolveAssemblyTypeIndex;
        private static readonly LuaCSFunction ZLuaTypeOfCallback = ZLuaTypeOf;
        private static readonly LuaCSFunction ZLuaCreateSignatureCallback = ZLuaCreateSignature;
        private static readonly LuaCSFunction ZLuaGetMethodCallback = ZLuaGetMethod;
        private static readonly LuaCSFunction ZLuaRegisterMethodCallback = ZLuaRegisterMethod;
        private static readonly LuaCSFunction ZLuaMakeGenericTypeCallback = ZLuaMakeGenericType;
        private static readonly LuaCSFunction ZLuaMakeSzArrayTypeCallback = ZLuaMakeSzArrayType;
        private static readonly LuaCSFunction ZLuaMakeMdArrayTypeCallback = ZLuaMakeMdArrayType;
        private static readonly LuaCSFunction ZLuaNewSzArrayByElementTypeCallback = ZLuaNewSzArrayByElementType;
        private static readonly LuaCSFunction ZLuaNewSzArrayBySzArrayTypeCallback = ZLuaNewSzArrayBySzArrayType;
        private static readonly LuaCSFunction ZLuaNewMdArrayByMdArrayTypeCallback = ZLuaNewMdArrayByMdArrayType;
        private static readonly LuaCSFunction ZLuaNewMdArrayBySpecCallback = ZLuaNewMdArrayBySpec;
        private static readonly LuaCSFunction ZLuaToDelegateCallback = ZLuaToDelegate;
        private static readonly LuaCSFunction ZLuaToUserDataCallback = ZLuaToUserData;
        private static readonly LuaCSFunction ArrayInstanceLenCallback = ArrayInstanceLen;
        private static readonly LuaCSFunction DelegateInstanceCallCallback = DelegateInstanceCall;
        private static readonly LuaCSFunction TypeTableToStringCallback = TypeTableToString;
        private static readonly LuaCSFunction EnumCtorCallback = InvokeEnumCtor;
        private static readonly LuaCSFunction EnumCallCallback = InvokeEnumCall;
        private static readonly LuaCSFunction EnumInstanceToStringCallback = EnumInstanceToString;
        private static readonly LuaCSFunction StructDefaultCallback = InvokeStructDefault;

        private static int _nextAssemblyId = 1;
        private static int _nextTypeId = 1;

        private readonly LuaEnv _luaEnv;

        public LuaManagerObject(LuaEnv luaEnv)
        {
            _luaEnv = luaEnv ?? throw new ArgumentNullException(nameof(luaEnv));
            IntPtr luaState = luaEnv.LuaState;
            int oldTop = LuaDll.lua_gettop(luaState);
            try
            {
                EnsureCSharpRoot(luaState);
            }
            finally
            {
                LuaDll.lua_settop(luaState, oldTop);
            }
        }

        public void RegisterZLuaApi()
        {
            IntPtr luaState = _luaEnv.LuaState;
            LuaDllExtension.RegisterCallback(luaState, "__zlua_typeof", ZLuaTypeOfCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_create_signature", ZLuaCreateSignatureCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_get_method", ZLuaGetMethodCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_register_method", ZLuaRegisterMethodCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_make_generic_type", ZLuaMakeGenericTypeCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_make_szarray_type", ZLuaMakeSzArrayTypeCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_make_mdarray_type", ZLuaMakeMdArrayTypeCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_new_szarray_by_element_type", ZLuaNewSzArrayByElementTypeCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_new_szarray_by_szarray_type", ZLuaNewSzArrayBySzArrayTypeCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_new_mdarray_by_mdarray_type", ZLuaNewMdArrayByMdArrayTypeCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_new_mdarray_by_spec", ZLuaNewMdArrayBySpecCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_to_delegate", ZLuaToDelegateCallback);
            LuaDllExtension.RegisterCallback(luaState, "__zlua_to_user_data", ZLuaToUserDataCallback);
            TypeMemberLuaIndexer.EnsureLoaded(luaState);
        }

        public void RegisterType(Type type)
        {
            IntPtr luaState = _luaEnv.LuaState;
            int oldTop = LuaDll.lua_gettop(luaState);
            try
            {
                GetCSharpRoot(luaState); // stack top: CSharp table
                PushAssemblyTable(luaState, type.Assembly); // stack top: assembly table
                PushInternedTypeTable(luaState, type);
                RawSetField(luaState, -2, GetLuaTypeFullName(type));
                LuaDll.lua_pop(luaState, 1); // pop assembly table
            }
            finally
            {
                LuaDll.lua_settop(luaState, oldTop);
            }
        }

        private static void EnsureCSharpRoot(IntPtr luaState)
        {
            LuaDataType csharpType = LuaDll.lua_getglobal(luaState, "CSharp");
            if (csharpType != LuaDataType.Table)
            {
                LuaDll.lua_pop(luaState, 1);
                LuaDll.lua_createtable(luaState, 0, 8);
                LuaDll.lua_pushvalue(luaState, -1);
                LuaDll.lua_setglobal(luaState, "CSharp");
            }

            // CSharp.__index => 程序集懒注册，Lua 访问形式：CSharp["Assembly-CSharp"].Demo
            LuaDll.lua_createtable(luaState, 0, 1);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(CSharpIndexCallback);
            CallbackRefs.Add(CSharpIndexCallback);
            LuaDll.lua_pushcfunction(luaState, fn);
            LuaDll.lua_setfield(luaState, -2, "__index");
            LuaDll.lua_setmetatable(luaState, -2);
        }

        private static void GetCSharpRoot(IntPtr luaState)
        {
            LuaDataType csharpType = LuaDll.lua_getglobal(luaState, "CSharp");
            Debug.Assert(csharpType == LuaDataType.Table);
        }

        private static void PushAssemblyTable(IntPtr luaState, Assembly assembly)
        {
            string luaAssemblyName = NormalizeAssemblyName(assembly.GetName().Name);
            LuaDataType existsType = RawGetField(luaState, -1, luaAssemblyName);
            if (existsType == LuaDataType.Table)
            {
                return; // 栈顶就是已存在的 assembly table
            }
            LuaDll.lua_pop(luaState, 1);

            int assemblyId = GetOrCreateAssemblyId(assembly);
            LuaDll.lua_createtable(luaState, 0, 16); // assembly table
            LuaDll.lua_createtable(luaState, 0, 1);  // metatable
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(AssemblyTypeIndexCallback);
            CallbackRefs.Add(AssemblyTypeIndexCallback);
            LuaDll.lua_pushinteger(luaState, assemblyId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
            LuaDll.lua_setfield(luaState, -2, "__index");
            LuaDll.lua_setmetatable(luaState, -2);

            LuaDll.lua_pushvalue(luaState, -1);
            LuaDll.lua_setfield(luaState, -3, luaAssemblyName);
        }

        private static void PushTypeTable(IntPtr luaState, Type type)
        {
            TypeMethodRegistration.EnsureMethodAliasKeysValid(type);

            if (type.IsEnum)
            {
                PushEnumTypeTable(luaState, type);
                return;
            }

            if (ValueTypeMarshaling.IsStructType(type))
            {
                PushStructTypeTable(luaState, type);
                return;
            }

            PushClassTypeTable(luaState, type);
        }

        private static void PushClassTypeTable(IntPtr luaState, Type type)
        {
            int typeId = GetOrCreateTypeId(type);
            LuaDll.lua_createtable(luaState, 0, 16);
            int typeTableIndex = LuaDll.lua_absindex(luaState, -1);

            LuaDll.lua_pushstring(luaState, NormalizeAssemblyName(type.Assembly.GetName().Name));
            LuaDll.lua_setfield(luaState, typeTableIndex, "__assembly");
            LuaDll.lua_pushstring(luaState, GetLuaTypeFullName(type));
            LuaDll.lua_setfield(luaState, typeTableIndex, "__fullname");
            LuaDll.lua_pushstring(luaState, type.Name);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__name");
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__typeid");

            int staticMetatableIndex = LuaDll.lua_gettop(luaState) + 1;
            LuaDll.lua_createtable(luaState, 0, 16);

            CreateMemberTablesOnStack(luaState, out int staticMethodTableIndex, out int staticGetterTableIndex, out int staticSetterTableIndex);
            TypeFieldRegistration.RegisterFields(type, typeId);
            TypePropertyRegistration.RegisterProperties(type, typeId);
            PopulateStaticMemberTables(
                luaState,
                type,
                typeId,
                staticMethodTableIndex,
                staticGetterTableIndex,
                staticSetterTableIndex);
            TypeMethodRegistration.RegisterConstructors(luaState, staticMetatableIndex, type);

            PushInstanceMetatable(luaState, type, typeTableIndex);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__instance_mt");

            AttachStaticMetatable(
                luaState,
                staticMetatableIndex,
                staticMethodTableIndex,
                staticGetterTableIndex,
                staticSetterTableIndex,
                typeId);

            LuaDll.lua_pop(luaState, 3);

            LuaDll.lua_setmetatable(luaState, typeTableIndex);

            // 清理注册过程中的临时栈项，保证栈顶是 type table 本身。
            LuaDll.lua_settop(luaState, typeTableIndex);
        }

        private static void PushStructTypeTable(IntPtr luaState, Type structType)
        {
            int typeId = GetOrCreateTypeId(structType);
            LuaDll.lua_createtable(luaState, 0, 16);
            int typeTableIndex = LuaDll.lua_absindex(luaState, -1);

            WriteTypeMetadata(luaState, typeTableIndex, structType, typeId);
            LuaDll.lua_pushboolean(luaState, 1);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__struct");

            int staticMetatableIndex = LuaDll.lua_gettop(luaState) + 1;
            LuaDll.lua_createtable(luaState, 0, 16);

            CreateMemberTablesOnStack(luaState, out int staticMethodTableIndex, out int staticGetterTableIndex, out int staticSetterTableIndex);
            TypeFieldRegistration.RegisterFields(structType, typeId);
            TypePropertyRegistration.RegisterProperties(structType, typeId);
            PopulateStaticMemberTables(
                luaState,
                structType,
                typeId,
                staticMethodTableIndex,
                staticGetterTableIndex,
                staticSetterTableIndex);
            TypeMethodRegistration.RegisterConstructors(luaState, staticMetatableIndex, structType);

            CallbackRefs.Add(StructDefaultCallback);
            IntPtr defaultFn = Marshal.GetFunctionPointerForDelegate(StructDefaultCallback);
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_pushcclosure(luaState, defaultFn, 1);
            LuaDll.lua_setfield(luaState, staticMetatableIndex, "_default");

            AttachStaticMetatable(
                luaState,
                staticMetatableIndex,
                staticMethodTableIndex,
                staticGetterTableIndex,
                staticSetterTableIndex,
                typeId);
            LuaDll.lua_pop(luaState, 3);

            PushValueTypeInstanceMetatable(luaState, structType, typeTableIndex, null);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__instance_mt");

            LuaDll.lua_setmetatable(luaState, typeTableIndex);
            LuaDll.lua_settop(luaState, typeTableIndex);
        }

        private static void PushEnumTypeTable(IntPtr luaState, Type enumType)
        {
            int typeId = GetOrCreateTypeId(enumType);
            LuaDll.lua_createtable(luaState, 0, 8);
            int typeTableIndex = LuaDll.lua_absindex(luaState, -1);

            WriteTypeMetadata(luaState, typeTableIndex, enumType, typeId);
            LuaDll.lua_pushboolean(luaState, 1);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__enum");

            int staticMetatableIndex = LuaDll.lua_gettop(luaState) + 1;
            LuaDll.lua_createtable(luaState, 0, 16);

            CreateMemberTablesOnStack(luaState, out int staticMethodTableIndex, out int staticGetterTableIndex, out int staticSetterTableIndex);
            RegisterEnumConstants(luaState, staticMethodTableIndex, enumType);

            CallbackRefs.Add(EnumCtorCallback);
            IntPtr ctorFn = Marshal.GetFunctionPointerForDelegate(EnumCtorCallback);
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_pushcclosure(luaState, ctorFn, 1);
            LuaDll.lua_setfield(luaState, staticMetatableIndex, "_ctor");

            CallbackRefs.Add(EnumCallCallback);
            IntPtr callFn = Marshal.GetFunctionPointerForDelegate(EnumCallCallback);
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_pushcclosure(luaState, callFn, 1);
            LuaDll.lua_setfield(luaState, staticMetatableIndex, "__call");

            AttachStaticMetatable(
                luaState,
                staticMetatableIndex,
                staticMethodTableIndex,
                staticGetterTableIndex,
                staticSetterTableIndex,
                typeId);
            LuaDll.lua_pop(luaState, 3);

            PushValueTypeInstanceMetatable(luaState, enumType, typeTableIndex, EnumInstanceToStringCallback);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__instance_mt");

            LuaDll.lua_setmetatable(luaState, typeTableIndex);
            LuaDll.lua_settop(luaState, typeTableIndex);
        }

        private static void CreateMemberTablesOnStack(
            IntPtr luaState,
            out int methodTableIndex,
            out int getterTableIndex,
            out int setterTableIndex)
        {
            LuaDll.lua_createtable(luaState, 0, 16);
            methodTableIndex = LuaDll.lua_absindex(luaState, -1);
            LuaDll.lua_createtable(luaState, 0, 16);
            getterTableIndex = LuaDll.lua_absindex(luaState, -1);
            LuaDll.lua_createtable(luaState, 0, 8);
            setterTableIndex = LuaDll.lua_absindex(luaState, -1);
        }

        private static void PopulateStaticMemberTables(
            IntPtr luaState,
            Type type,
            int typeId,
            int methodTableIndex,
            int getterTableIndex,
            int setterTableIndex)
        {
            TypeMethodRegistration.RegisterStaticMethods(luaState, methodTableIndex, type);
            TypeFieldRegistration.BindGetterTable(luaState, getterTableIndex, typeId, isStatic: true);
            TypeFieldRegistration.BindSetterTable(luaState, setterTableIndex, typeId, isStatic: true);
            TypePropertyRegistration.BindGetterTable(luaState, getterTableIndex, typeId, isStatic: true);
            TypePropertyRegistration.BindSetterTable(luaState, setterTableIndex, typeId, isStatic: true);
            TypeEventRegistration.RegisterEvents(luaState, methodTableIndex, type, typeId);
        }

        private static void PopulateInstanceMemberTables(
            IntPtr luaState,
            Type type,
            int typeId,
            int methodTableIndex,
            int getterTableIndex,
            int setterTableIndex)
        {
            TypeMethodRegistration.RegisterInstanceMethods(luaState, methodTableIndex, type);
            TypeFieldRegistration.BindGetterTable(luaState, getterTableIndex, typeId, isStatic: false);
            TypeFieldRegistration.BindSetterTable(luaState, setterTableIndex, typeId, isStatic: false);
            TypePropertyRegistration.BindGetterTable(luaState, getterTableIndex, typeId, isStatic: false);
            TypePropertyRegistration.BindSetterTable(luaState, setterTableIndex, typeId, isStatic: false);
            TypeEventRegistration.BindInstanceEventsToMethodTable(luaState, methodTableIndex, typeId);
        }

        private static void AttachStaticMetatable(
            IntPtr luaState,
            int staticMetatableIndex,
            int methodTableIndex,
            int getterTableIndex,
            int setterTableIndex,
            int typeId)
        {
            RetainMethodTableRef(luaState, StaticMethodTableRefsByTypeId, typeId, methodTableIndex);

            TypeMemberLuaIndexer.BindStaticMetatable(
                luaState,
                staticMetatableIndex,
                methodTableIndex,
                getterTableIndex,
                setterTableIndex);

            CallbackRefs.Add(TypeTableToStringCallback);
            IntPtr toStringFn = Marshal.GetFunctionPointerForDelegate(TypeTableToStringCallback);
            LuaDll.lua_pushcfunction(luaState, toStringFn);
            LuaDll.lua_setfield(luaState, staticMetatableIndex, "__tostring");
        }

        private static void WriteTypeMetadata(IntPtr luaState, int typeTableIndex, Type type, int typeId)
        {
            LuaDll.lua_pushstring(luaState, NormalizeAssemblyName(type.Assembly.GetName().Name));
            LuaDll.lua_setfield(luaState, typeTableIndex, "__assembly");
            LuaDll.lua_pushstring(luaState, GetLuaTypeFullName(type));
            LuaDll.lua_setfield(luaState, typeTableIndex, "__fullname");
            LuaDll.lua_pushstring(luaState, type.Name);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__name");
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__typeid");
        }

        private static void PushValueTypeInstanceMetatable(IntPtr luaState, Type type, int typeTableIndex, LuaCSFunction toStringCallback)
        {
            LuaDll.lua_createtable(luaState, 0, 8);
            int mtIndex = LuaDll.lua_absindex(luaState, -1);

            LuaDll.lua_pushvalue(luaState, typeTableIndex);
            LuaDll.lua_setfield(luaState, mtIndex, "__type");

            int instanceTypeId = GetOrCreateTypeId(type);

            CreateMemberTablesOnStack(luaState, out int methodTableIndex, out int getterTableIndex, out int setterTableIndex);
            PopulateInstanceMemberTables(luaState, type, instanceTypeId, methodTableIndex, getterTableIndex, setterTableIndex);
            RetainMethodTableRef(luaState, InstanceMethodTableRefsByTypeId, instanceTypeId, methodTableIndex);

            TypeMemberLuaIndexer.BindInstanceMetatable(
                luaState,
                mtIndex,
                methodTableIndex,
                getterTableIndex,
                setterTableIndex);
            LuaDll.lua_pop(luaState, 3);

            LuaCSFunction gcCb = ReleaseUserData;
            CallbackRefs.Add(gcCb);
            IntPtr gcFn = Marshal.GetFunctionPointerForDelegate(gcCb);
            LuaDll.lua_pushcfunction(luaState, gcFn);
            LuaDll.lua_setfield(luaState, mtIndex, "__gc");

            if (toStringCallback != null)
            {
                CallbackRefs.Add(toStringCallback);
                IntPtr instanceToStringFn = Marshal.GetFunctionPointerForDelegate(toStringCallback);
                LuaDll.lua_pushinteger(luaState, instanceTypeId);
                LuaDll.lua_pushcclosure(luaState, instanceToStringFn, 1);
                LuaDll.lua_setfield(luaState, mtIndex, "__tostring");
            }
        }

        private static void RegisterEnumConstants(IntPtr luaState, int methodTableIndex, Type enumType)
        {
            Type underlying = Enum.GetUnderlyingType(enumType);
            FieldInfo[] fields = enumType.GetFields(BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly);
            for (int i = 0; i < fields.Length; i++)
            {
                FieldInfo field = fields[i];
                if (!field.IsLiteral)
                {
                    continue;
                }

                ValueTypeMarshaling.PushUnderlyingInteger(luaState, underlying, field.GetRawConstantValue());
                LuaDll.lua_setfield(luaState, methodTableIndex, field.Name);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeEnumCtor(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type enumType) || !enumType.IsEnum)
                {
                    return LuaDllExtension.error(luaState, $"zlua: enum type id {typeId} not found");
                }

                if (LuaDll.lua_gettop(luaState) < 1)
                {
                    return LuaDllExtension.error(luaState, $"zlua: {GetLuaTypeFullName(enumType)}._ctor expects underlying integer value");
                }

                Type underlying = Enum.GetUnderlyingType(enumType);
                if (!ValueTypeMarshaling.TryReadUnderlyingInteger(luaState, 1, underlying, out object rawValue))
                {
                    return LuaDllExtension.error(luaState, $"zlua: {GetLuaTypeFullName(enumType)}._ctor expects underlying integer value");
                }

                object enumValue;
                try
                {
                    enumValue = Enum.ToObject(enumType, rawValue);
                }
                catch (Exception ex)
                {
                    return LuaDllExtension.error(luaState, $"zlua: invalid enum value for {GetLuaTypeFullName(enumType)}: {ex.Message}");
                }

                return PushConstructorInstance(luaState, enumValue, enumType);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua enum ctor error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeEnumCall(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type enumType) || !enumType.IsEnum)
                {
                    return LuaDllExtension.error(luaState, $"zlua: enum type id {typeId} not found");
                }

                if (LuaDll.lua_gettop(luaState) < 2)
                {
                    return LuaDllExtension.error(luaState, $"zlua: {GetLuaTypeFullName(enumType)} expects underlying integer value");
                }

                Type underlying = Enum.GetUnderlyingType(enumType);
                if (!ValueTypeMarshaling.TryReadUnderlyingInteger(luaState, 2, underlying, out object rawValue))
                {
                    return LuaDllExtension.error(luaState, $"zlua: {GetLuaTypeFullName(enumType)} expects underlying integer value");
                }

                object enumValue;
                try
                {
                    enumValue = Enum.ToObject(enumType, rawValue);
                }
                catch (Exception ex)
                {
                    return LuaDllExtension.error(luaState, $"zlua: invalid enum value for {GetLuaTypeFullName(enumType)}: {ex.Message}");
                }

                return ValueTypeMarshaling.PushBoxedInstance(luaState, enumValue, 1);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua enum call error: {ex}");
            }
        }

        internal static int PushConstructorInstance(IntPtr luaState, object instance, Type type)
        {
            PushInternedTypeTable(luaState, type);
            int typeTableIndex = LuaDll.lua_gettop(luaState);
            int pushed = ValueTypeMarshaling.PushBoxedInstance(luaState, instance, typeTableIndex);
            LuaDll.lua_remove(luaState, typeTableIndex);
            return pushed;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStructDefault(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type structType) || !ValueTypeMarshaling.IsStructType(structType))
                {
                    return LuaDllExtension.error(luaState, $"zlua: struct type id {typeId} not found");
                }

                if (LuaDll.lua_gettop(luaState) != 0)
                {
                    return LuaDllExtension.error(luaState, $"zlua: {GetLuaTypeFullName(structType)}._default expects no arguments");
                }

                object instance = Activator.CreateInstance(structType);
                return PushConstructorInstance(luaState, instance, structType);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua struct default error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int EnumInstanceToString(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type enumType) || !enumType.IsEnum)
                {
                    return LuaDllExtension.error(luaState, $"zlua: enum type id {typeId} not found");
                }

                if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, 1, out object enumValue) || enumValue == null)
                {
                    return LuaDllExtension.error(luaState, "zlua: invalid enum userdata");
                }

                object raw = Convert.ChangeType(enumValue, Enum.GetUnderlyingType(enumType));
                LuaDll.lua_pushstring(luaState, GetLuaTypeFullName(enumType) + "(" + Convert.ToInt64(raw) + ")");
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua enum tostring error: {ex}");
            }
        }

        private static void PushInstanceMetatable(IntPtr luaState, Type type, int typeTableIndex)
        {
            LuaDll.lua_createtable(luaState, 0, 16);
            int mtIndex = LuaDll.lua_absindex(luaState, -1);

            LuaDll.lua_pushvalue(luaState, typeTableIndex);
            LuaDll.lua_setfield(luaState, mtIndex, "__type");

            int instanceTypeId = GetOrCreateTypeId(type);

            CreateMemberTablesOnStack(luaState, out int methodTableIndex, out int getterTableIndex, out int setterTableIndex);
            PopulateInstanceMemberTables(luaState, type, instanceTypeId, methodTableIndex, getterTableIndex, setterTableIndex);
            RetainMethodTableRef(luaState, InstanceMethodTableRefsByTypeId, instanceTypeId, methodTableIndex);

            TypeMemberLuaIndexer.BindInstanceMetatable(
                luaState,
                mtIndex,
                methodTableIndex,
                getterTableIndex,
                setterTableIndex);
            LuaDll.lua_pop(luaState, 3);

            if (type.IsArray && type.GetArrayRank() == 1)
            {
                CallbackRefs.Add(ArrayInstanceLenCallback);
                IntPtr lenFn = Marshal.GetFunctionPointerForDelegate(ArrayInstanceLenCallback);
                LuaDll.lua_pushcfunction(luaState, lenFn);
                LuaDll.lua_setfield(luaState, mtIndex, "__len");
            }

            if (typeof(Delegate).IsAssignableFrom(type))
            {
                CallbackRefs.Add(DelegateInstanceCallCallback);
                IntPtr callFn = Marshal.GetFunctionPointerForDelegate(DelegateInstanceCallCallback);
                LuaDll.lua_pushcfunction(luaState, callFn);
                LuaDll.lua_setfield(luaState, mtIndex, "__call");
            }

            LuaCSFunction gcCb = ReleaseUserData;
            CallbackRefs.Add(gcCb);
            IntPtr gcFn = Marshal.GetFunctionPointerForDelegate(gcCb);
            LuaDll.lua_pushcfunction(luaState, gcFn);
            LuaDll.lua_setfield(luaState, mtIndex, "__gc");
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ArrayInstanceLen(IntPtr luaState)
        {
            try
            {
                if (!ValueTypeMarshaling.TryGetBoxedTarget(luaState, 1, out object target) || !(target is Array array))
                {
                    return LuaDllExtension.error(luaState, "zlua: __len expects szarray userdata");
                }

                if (array.Rank != 1)
                {
                    return LuaDllExtension.error(luaState, "zlua: __len only supported for rank-1 arrays");
                }

                LuaDll.lua_pushinteger(luaState, array.Length);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua array __len error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DelegateInstanceCall(IntPtr luaState)
        {
            try
            {
                if (!TryGetUserDataTarget(luaState, 1, out object target) || !(target is Delegate del))
                {
                    return LuaDllExtension.error(luaState, "zlua: __call expects delegate userdata");
                }

                MethodInfo invokeMethod = del.GetType().GetMethod("Invoke");
                ParameterInfo[] parameters = invokeMethod.GetParameters();
                int luaArgCount = LuaDll.lua_gettop(luaState) - 1;
                if (luaArgCount != parameters.Length)
                {
                    return LuaDllExtension.error(luaState,
                        $"zlua: delegate invoke expects {parameters.Length} argument(s), got {luaArgCount}");
                }

                object[] args = new object[parameters.Length];
                for (int i = 0; i < parameters.Length; i++)
                {
                    args[i] = TypeMethodRegistration.ReadArgumentValue(luaState, i + 2, parameters[i].ParameterType);
                }

                object ret = del.DynamicInvoke(args);
                return TypeMethodRegistration.PushReturnValue(luaState, invokeMethod.ReturnType, ret);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua delegate __call error: {ex.InnerException?.Message ?? ex.Message}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaToDelegate(IntPtr luaState)
        {
            try
            {
                if (LuaDll.lua_type(luaState, 1) != LuaDataType.Function)
                {
                    return LuaDllExtension.error(luaState, "zlua.to_delegate expects Lua function");
                }

                if (!TryResolveTypeArg(luaState, 2, out Type delegateType))
                {
                    return LuaDllExtension.error(luaState, "zlua.to_delegate expects closed delegate type");
                }

                if (!typeof(Delegate).IsAssignableFrom(delegateType))
                {
                    return LuaDllExtension.error(luaState, "zlua.to_delegate expects delegate type");
                }

                int funcRef = LuaDelegateBinder.CreateFunctionRef(luaState, 1);
                try
                {
                    Delegate del = LuaDelegateBinder.Create(LuaMonoAppDomain.LuaEnv, delegateType, funcRef);
                    return PushConstructorInstance(luaState, del, delegateType);
                }
                catch
                {
                    LuaDll.luaL_unref(luaState, LuaConsts.LuaRegistryIndex, funcRef);
                    throw;
                }
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua to_delegate error: {ex.Message}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaToUserData(IntPtr luaState)
        {
            try
            {
                return OpaqueMarshaling.ToUserData(luaState, 1);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua to_user_data error: {ex.Message}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int TypeTableToString(IntPtr luaState)
        {
            LuaDataType fullNameType = RawGetField(luaState, 1, "__fullname");
            if (fullNameType != LuaDataType.String)
            {
                LuaDll.lua_pop(luaState, 1);
                LuaDll.lua_pushstring(luaState, string.Empty);
            }

            return 1;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ResolveAssemblyIndex(IntPtr luaState)
        {
            try
            {
                string luaAssemblyName = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrWhiteSpace(luaAssemblyName))
                {
                    return 0;
                }

                LuaDataType existsType = RawGetField(luaState, 1, luaAssemblyName);
                if (existsType == LuaDataType.Table)
                {
                    return 1;
                }
                LuaDll.lua_pop(luaState, 1);

                Assembly assembly = ResolveAssembly(luaAssemblyName);
                if (assembly == null)
                {
                    return 0;
                }

                int assemblyId = GetOrCreateAssemblyId(assembly);
                LuaDll.lua_createtable(luaState, 0, 16); // assembly table
                LuaDll.lua_createtable(luaState, 0, 1);  // metatable
                IntPtr fn = Marshal.GetFunctionPointerForDelegate(AssemblyTypeIndexCallback);
                CallbackRefs.Add(AssemblyTypeIndexCallback);
                LuaDll.lua_pushinteger(luaState, assemblyId);
                LuaDll.lua_pushcclosure(luaState, fn, 1);
                LuaDll.lua_setfield(luaState, -2, "__index");
                LuaDll.lua_setmetatable(luaState, -2);

                LuaDll.lua_pushvalue(luaState, -1);
                RawSetField(luaState, 1, luaAssemblyName);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua ResolveAssemblyIndex error: {ex}");
            }
        }

        internal static LuaDataType RawGetFieldPublic(IntPtr luaState, int tableIndex, string key)
        {
            return RawGetField(luaState, tableIndex, key);
        }

        private static LuaDataType RawGetField(IntPtr luaState, int tableIndex, string key)
        {
            int absIndex = LuaDll.lua_absindex(luaState, tableIndex);
            LuaDll.lua_pushstring(luaState, key);
            return LuaDll.lua_rawget(luaState, absIndex);
        }

        private static void RawSetField(IntPtr luaState, int tableIndex, string key)
        {
            int absIndex = LuaDll.lua_absindex(luaState, tableIndex);
            LuaDll.lua_pushstring(luaState, key);
            LuaDll.lua_rawset(luaState, absIndex);
        }

        private static string BuildParameterSignature(IReadOnlyList<Type> parameterTypes)
        {
            if (parameterTypes == null || parameterTypes.Count == 0)
            {
                return "()";
            }

            return "(" + string.Join(",", parameterTypes.Select(GetLuaTypeFullName)) + ")";
        }

        internal static string FormatParameterSignature(ParameterInfo[] parameters)
        {
            if (parameters == null || parameters.Length == 0)
            {
                return "()";
            }

            return "(" + string.Join(",", parameters.Select(p => GetLuaTypeFullName(p.ParameterType))) + ")";
        }

        private static bool TryResolveTargetType(IntPtr luaState, int index, out Type type)
        {
            type = null;
            LuaDataType luaType = LuaDll.lua_type(luaState, index);
            if (luaType == LuaDataType.Table)
            {
                return TryResolveTypeFromTypeTable(luaState, index, out type);
            }

            if (luaType == LuaDataType.UserData)
            {
                if (TryGetUserDataTarget(luaState, index, out object target) && target != null)
                {
                    type = target.GetType();
                    return true;
                }
            }

            return false;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaGetMethod(IntPtr luaState)
        {
            try
            {
                if (LuaDll.lua_gettop(luaState) < 4)
                {
                    return LuaDllExtension.error(luaState, "zlua.get_method expects (target, methodName, signature, is_static)");
                }

                if (!TryResolveTargetType(luaState, 1, out Type targetType))
                {
                    return LuaDllExtension.error(luaState, "zlua.get_method expects userdata or type table as target");
                }

                string methodName = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrWhiteSpace(methodName))
                {
                    return LuaDllExtension.error(luaState, "zlua.get_method expects method name");
                }

                string signature = LuaDllExtension.tostring(luaState, 3);
                if (signature == null)
                {
                    return LuaDllExtension.error(luaState, "zlua.get_method expects signature string");
                }

                bool isStatic = LuaDll.lua_toboolean(luaState, 4) != 0;

                int oldTop = LuaDll.lua_gettop(luaState);
                PushInternedTypeTable(luaState, targetType);
                LuaDll.lua_settop(luaState, oldTop);

                if (!TypeMethodRegistration.TryFindMethodByParameterSignature(
                        targetType,
                        methodName,
                        signature,
                        isStatic,
                        out MethodInfo method))
                {
                    return LuaDllExtension.error(
                        luaState,
                        $"zlua: no overload for {GetLuaTypeFullName(targetType)}.{methodName} matching {signature}");
                }

                TypeMethodRegistration.PushRegisteredMethodClosure(luaState, method, isStatic);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua get_method error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaRegisterMethod(IntPtr luaState)
        {
            try
            {
                if (LuaDll.lua_gettop(luaState) < 3)
                {
                    return LuaDllExtension.error(
                        luaState,
                        "zlua.register_method expects (static_class_mt_or_obj, aliasName, methodOrClosure)");
                }

                string aliasName = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrWhiteSpace(aliasName))
                {
                    return LuaDllExtension.error(luaState, "zlua.register_method expects alias name");
                }

                LuaDataType closureType = LuaDll.lua_type(luaState, 3);
                if (closureType != LuaDataType.Function)
                {
                    return LuaDllExtension.error(luaState, "zlua.register_method expects callable closure");
                }

                int methodTableRef;
                LuaDataType targetLuaType = LuaDll.lua_type(luaState, 1);
                if (targetLuaType == LuaDataType.Table)
                {
                    if (!TryResolveTypeFromTypeTable(luaState, 1, out Type clrType))
                    {
                        return LuaDllExtension.error(
                            luaState,
                            "zlua.register_method expects type table as first argument");
                    }

                    int oldTop = LuaDll.lua_gettop(luaState);
                    PushInternedTypeTable(luaState, clrType);
                    LuaDll.lua_settop(luaState, oldTop);

                    if (!TryResolveStaticMethodTableRef(luaState, 1, out methodTableRef))
                    {
                        return LuaDllExtension.error(
                            luaState,
                            $"zlua: static method table not found for {GetLuaTypeFullName(clrType)}");
                    }
                }
                else if (targetLuaType == LuaDataType.UserData)
                {
                    if (!TryGetUserDataTarget(luaState, 1, out object target) || target == null)
                    {
                        return LuaDllExtension.error(
                            luaState,
                            "zlua.register_method expects userdata instance");
                    }

                    int oldTop = LuaDll.lua_gettop(luaState);
                    PushInternedTypeTable(luaState, target.GetType());
                    LuaDll.lua_settop(luaState, oldTop);

                    if (!TryResolveInstanceMethodTableRef(luaState, 1, out methodTableRef))
                    {
                        return LuaDllExtension.error(
                            luaState,
                            $"zlua: instance method table not found for {GetLuaTypeFullName(target.GetType())}");
                    }
                }
                else
                {
                    return LuaDllExtension.error(
                        luaState,
                        "zlua.register_method expects type table or userdata instance");
                }

                if (!TryRegisterMethodAlias(luaState, methodTableRef, aliasName, closureStackIndex: 3))
                {
                    return LuaDllExtension.error(luaState, $"zlua: method alias already exists: {aliasName}");
                }

                return 0;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua register_method error: {ex}");
            }
        }

        private static void RetainMethodTableRef(
            IntPtr luaState,
            Dictionary<int, int> refsByTypeId,
            int typeId,
            int methodTableIndex)
        {
            if (refsByTypeId.ContainsKey(typeId))
            {
                return;
            }

            int absIndex = LuaDll.lua_absindex(luaState, methodTableIndex);
            if (LuaDll.lua_type(luaState, absIndex) != LuaDataType.Table)
            {
                throw new InvalidOperationException(
                    $"zlua: expected methodTable for type id {typeId}, got {LuaDll.lua_type(luaState, absIndex)}");
            }

            LuaDll.lua_pushvalue(luaState, absIndex);
            refsByTypeId[typeId] = LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
        }

        private static bool TryReadTypeIdFromTypeTable(IntPtr luaState, int typeTableIndex, out int typeId)
        {
            typeId = 0;
            LuaDataType idType = RawGetField(luaState, typeTableIndex, "__typeid");
            if (idType != LuaDataType.Number)
            {
                LuaDll.lua_pop(luaState, 1);
                return false;
            }

            typeId = (int)LuaDll.lua_tointeger(luaState, -1);
            LuaDll.lua_pop(luaState, 1);
            return Types.ContainsKey(typeId);
        }

        private static bool TryResolveStaticMethodTableRef(IntPtr luaState, int typeTableIndex, out int methodTableRef)
        {
            methodTableRef = 0;
            if (!TryReadTypeIdFromTypeTable(luaState, typeTableIndex, out int typeId))
            {
                return false;
            }

            return StaticMethodTableRefsByTypeId.TryGetValue(typeId, out methodTableRef) && methodTableRef != 0;
        }

        private static bool TryResolveInstanceMethodTableRef(IntPtr luaState, int userdataIndex, out int methodTableRef)
        {
            methodTableRef = 0;
            if (LuaDll.lua_getmetatable(luaState, userdataIndex) == 0)
            {
                return false;
            }

            int metatableIndex = LuaDll.lua_absindex(luaState, -1);
            LuaDataType typeFieldType = RawGetField(luaState, metatableIndex, "__type");
            if (typeFieldType != LuaDataType.Table)
            {
                LuaDll.lua_pop(luaState, 1);
                return false;
            }

            int typeTableIndex = LuaDll.lua_absindex(luaState, -1);
            bool resolved = TryReadTypeIdFromTypeTable(luaState, typeTableIndex, out int typeId)
                && InstanceMethodTableRefsByTypeId.TryGetValue(typeId, out methodTableRef)
                && methodTableRef != 0;
            LuaDll.lua_pop(luaState, 2);
            return resolved;
        }

        private static bool TryRegisterMethodAlias(
            IntPtr luaState,
            int methodTableRef,
            string aliasName,
            int closureStackIndex)
        {
            LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, methodTableRef);
            int methodTableIndex = LuaDll.lua_absindex(luaState, -1);

            LuaDll.lua_pushstring(luaState, aliasName);
            if (LuaDll.lua_rawget(luaState, methodTableIndex) != LuaDataType.Nil)
            {
                LuaDll.lua_pop(luaState, 2);
                return false;
            }

            LuaDll.lua_pop(luaState, 1);
            LuaDll.lua_pushstring(luaState, aliasName);
            LuaDll.lua_pushvalue(luaState, closureStackIndex);
            LuaDll.lua_rawset(luaState, methodTableIndex);
            LuaDll.lua_pop(luaState, 1);
            return true;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ResolveAssemblyTypeIndex(IntPtr luaState)
        {
            try
            {
                int upvalueIndex = LuaConsts.LuaRegistryIndex - 1;
                int assemblyId = (int)LuaDll.lua_tointeger(luaState, upvalueIndex);
                if (!Assemblies.TryGetValue(assemblyId, out Assembly assembly))
                {
                    return 0;
                }

                string typeName = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrWhiteSpace(typeName))
                {
                    return 0;
                }

                if (!TryResolveType(assembly, typeName, out Type type))
                {
                    return 0;
                }

                TypeMethodRegistration.EnsureMethodAliasKeysValid(type);

                PushInternedTypeTable(luaState, type);
                LuaDll.lua_pushvalue(luaState, -1);
                RawSetField(luaState, 1, typeName); // cache in assembly table
                return 1;
            }
            catch (Exception ex)
            {
                // __index 回调约定栈：(assemblyTable, key)。注册失败时须丢弃部分压栈的临时表，避免污染 Lua 栈。
                LuaDll.lua_settop(luaState, 2);
                string message = ex is InvalidOperationException ? ex.Message : $"zlua ResolveAssemblyTypeIndex error: {ex}";
                return LuaDllExtension.error(luaState, message);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaTypeOf(IntPtr luaState)
        {
            try
            {
                // editor 原型阶段：type table 本身即可作为“类型对象”在 zlua helper 中继续使用
                if (LuaDll.lua_type(luaState, 1) == LuaDataType.Table)
                {
                    LuaDll.lua_pushvalue(luaState, 1);
                    return 1;
                }

                return LuaDllExtension.error(luaState, "zlua.typeof expects a csharp type table");
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua typeof error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaCreateSignature(IntPtr luaState)
        {
            try
            {
                int top = LuaDll.lua_gettop(luaState);
                List<Type> parameterTypes = new List<Type>(Math.Max(top, 0));
                for (int i = 1; i <= top; i++)
                {
                    if (!TryResolveTypeArg(luaState, i, out Type argType))
                    {
                        return LuaDllExtension.error(luaState, $"zlua.signature arg{i} is not a type");
                    }

                    parameterTypes.Add(argType);
                }

                string signature = BuildParameterSignature(parameterTypes);
                LuaDll.lua_pushstring(luaState, signature);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua signature error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeGenericType(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type genericType))
                {
                    return LuaDllExtension.error(luaState, "zlua.make_generic_type expects generic type table as first arg");
                }

                if (!genericType.IsGenericTypeDefinition)
                {
                    return LuaDllExtension.error(luaState, $"type is not a generic definition: {GetLuaTypeFullName(genericType)}");
                }

                int top = LuaDll.lua_gettop(luaState);
                Type[] genericArgs = new Type[Math.Max(top - 1, 0)];
                for (int i = 2; i <= top; i++)
                {
                    if (!TryResolveTypeArg(luaState, i, out Type argType))
                    {
                        return LuaDllExtension.error(luaState, $"generic arg {i - 1} is not a type");
                    }

                    genericArgs[i - 2] = argType;
                }

                if (genericArgs.Length != genericType.GetGenericArguments().Length)
                {
                    return LuaDllExtension.error(luaState,
                        $"generic arg count mismatch: expected {genericType.GetGenericArguments().Length}, got {genericArgs.Length}");
                }

                Type closedType = genericType.MakeGenericType(genericArgs);
                PushInternedTypeTable(luaState, closedType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua make_generic_type error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeSzArrayType(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type elementType))
                {
                    return LuaDllExtension.error(luaState, "zlua.make_szarray_type expects element type");
                }

                Type arrayType = elementType.MakeArrayType();
                PushInternedTypeTable(luaState, arrayType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua make_szarray_type error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaMakeMdArrayType(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type elementType))
                {
                    return LuaDllExtension.error(luaState, "zlua.make_mdarray_type expects element type");
                }

                int rank = (int)LuaDll.lua_tointeger(luaState, 2);
                if (rank < 1)
                {
                    return LuaDllExtension.error(luaState, "zlua.make_mdarray_type rank must be >= 1");
                }

                Type arrayType = elementType.MakeArrayType(rank);
                PushInternedTypeTable(luaState, arrayType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua make_mdarray_type error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewSzArrayByElementType(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type elementType))
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_element_type expects element type");
                }

                if (LuaDll.lua_isinteger(luaState, 2) == 0)
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_element_type expects integer length");
                }

                long length = LuaDll.lua_tointeger(luaState, 2);
                if (length < 0)
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_element_type length must be >= 0");
                }

                Array array = ArrayMarshaling.CreateSzArray(elementType, (int)length);
                return PushConstructorInstance(luaState, array, elementType.MakeArrayType());
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua new_szarray_by_element_type error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewSzArrayBySzArrayType(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type arrayType))
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_szarray_type expects szarray type table");
                }

                if (!ArrayMarshaling.IsSzArrayType(arrayType))
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_szarray_type expects rank-1 array type");
                }

                if (LuaDll.lua_isinteger(luaState, 2) == 0)
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_szarray_type expects integer length");
                }

                long length = LuaDll.lua_tointeger(luaState, 2);
                if (length < 0)
                {
                    return LuaDllExtension.error(luaState, "zlua.new_szarray_by_szarray_type length must be >= 0");
                }

                Type elementType = arrayType.GetElementType();
                Array array = ArrayMarshaling.CreateSzArray(elementType, (int)length);
                return PushConstructorInstance(luaState, array, arrayType);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua new_szarray_by_szarray_type error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewMdArrayByMdArrayType(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type arrayType))
                {
                    return LuaDllExtension.error(luaState, "zlua.new_mdarray_by_mdarray_type expects mdarray type table");
                }

                if (!ArrayMarshaling.IsMdArrayType(arrayType))
                {
                    return LuaDllExtension.error(luaState, "zlua.new_mdarray_by_mdarray_type expects array type");
                }

                int rank = arrayType.GetArrayRank();
                return CreateMdArrayInstance(luaState, arrayType.GetElementType(), arrayType, rank, 2, 3);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua new_mdarray_by_mdarray_type error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ZLuaNewMdArrayBySpec(IntPtr luaState)
        {
            try
            {
                if (!TryResolveTypeArg(luaState, 1, out Type elementType))
                {
                    return LuaDllExtension.error(luaState, "zlua.new_mdarray_by_spec expects element type");
                }

                if (!ArrayMarshaling.TryGetConsecutiveTableLength(luaState, 3, out int rank, out string rankError))
                {
                    return LuaDllExtension.error(luaState, $"zlua.new_mdarray_by_spec sizes: {rankError}");
                }

                if (rank < 1)
                {
                    return LuaDllExtension.error(luaState, "zlua.new_mdarray_by_spec rank must be >= 1");
                }

                Type arrayType = rank == 1 ? elementType.MakeArrayType() : elementType.MakeArrayType(rank);
                return CreateMdArrayInstance(luaState, elementType, arrayType, rank, 2, 3);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua new_mdarray_by_spec error: {ex}");
            }
        }

        private static int CreateMdArrayInstance(
            IntPtr luaState,
            Type elementType,
            Type arrayType,
            int rank,
            int lowboundsIndex,
            int sizesIndex)
        {
            if (!ArrayMarshaling.TryGetConsecutiveTableLength(luaState, lowboundsIndex, out int lowboundsLength, out string lowboundsError))
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* lowbounds: {lowboundsError}");
            }

            if (!ArrayMarshaling.TryGetConsecutiveTableLength(luaState, sizesIndex, out int sizesLength, out string sizesError))
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* sizes: {sizesError}");
            }

            if (lowboundsLength != rank)
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* lowbounds length must be {rank}");
            }

            if (sizesLength != rank)
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* sizes length must be {rank}");
            }

            if (!ArrayMarshaling.TryReadIntSequence(luaState, lowboundsIndex, rank, out int[] lowerBounds, out string readLowboundsError))
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* lowbounds: {readLowboundsError}");
            }

            if (!ArrayMarshaling.TryReadIntSequence(luaState, sizesIndex, rank, out int[] sizes, out string readSizesError))
            {
                LuaCallbackBoundary.Throw($"zlua.new_mdarray_* sizes: {readSizesError}");
            }

            for (int i = 0; i < sizes.Length; i++)
            {
                if (sizes[i] < 0)
                {
                    LuaCallbackBoundary.Throw("zlua.new_mdarray_* sizes must be >= 0");
                }
            }

            Array array = ArrayMarshaling.CreateMdArray(elementType, sizes, lowerBounds);
            return PushConstructorInstance(luaState, array, arrayType);
        }

        private static string ReadTypeNameFromTypeTable(IntPtr luaState, int index)
        {
            LuaDataType type = RawGetField(luaState, index, "__fullname");
            if (type != LuaDataType.String)
            {
                LuaDll.lua_pop(luaState, 1);
                return null;
            }
            string value = LuaDllExtension.tostring(luaState, -1);
            LuaDll.lua_pop(luaState, 1);
            return value;
        }

        private static string ReadAssemblyNameFromTypeTable(IntPtr luaState, int index)
        {
            LuaDataType type = RawGetField(luaState, index, "__assembly");
            if (type != LuaDataType.String)
            {
                LuaDll.lua_pop(luaState, 1);
                return null;
            }
            string value = LuaDllExtension.tostring(luaState, -1);
            LuaDll.lua_pop(luaState, 1);
            return value;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ReleaseUserData(IntPtr luaState)
        {
            return ValueTypeMarshaling.ReleaseBoxedInstance(luaState);
        }

        private static Assembly ResolveAssembly(string luaAssemblyName)
        {
            if (AssemblyByLuaName.TryGetValue(luaAssemblyName, out Assembly cached))
            {
                return cached;
            }

            if (string.Equals(luaAssemblyName, "mscorlib", StringComparison.Ordinal))
            {
                AssemblyByLuaName[luaAssemblyName] = Mscorlib;
                return Mscorlib;
            }

            foreach (Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                string normalized = NormalizeAssemblyName(assembly.GetName().Name);
                if (string.Equals(normalized, luaAssemblyName, StringComparison.Ordinal))
                {
                    AssemblyByLuaName[luaAssemblyName] = assembly;
                    return assembly;
                }
            }

            AssemblyByLuaName[luaAssemblyName] = null;
            return null;
        }

        private static int GetOrCreateAssemblyId(Assembly assembly)
        {
            string key = assembly.FullName;
            if (AssemblyIds.TryGetValue(key, out int id))
            {
                return id;
            }

            id = _nextAssemblyId++;
            AssemblyIds[key] = id;
            Assemblies[id] = assembly;
            return id;
        }

        private static bool TryResolveType(Assembly assembly, string luaTypeName, out Type type)
        {
            string cacheKey = assembly.FullName + "::" + luaTypeName;
            if (TypeCache.TryGetValue(cacheKey, out type))
            {
                return type != null;
            }

            type = assembly.GetType(luaTypeName, false);
            if (type != null)
            {
                TypeCache[cacheKey] = type;
                return true;
            }

            Type[] types;
            try
            {
                types = assembly.GetTypes();
            }
            catch (ReflectionTypeLoadException e)
            {
                types = e.Types;
            }

            for (int i = 0; i < types.Length; i++)
            {
                Type candidate = types[i];
                if (candidate != null && string.Equals(GetLuaTypeFullName(candidate), luaTypeName, StringComparison.Ordinal))
                {
                    TypeCache[cacheKey] = candidate;
                    type = candidate;
                    return true;
                }
            }

            TypeCache[cacheKey] = null;
            type = null;
            return false;
        }

        private static int GetOrCreateTypeId(Type type)
        {
            foreach (var kv in Types)
            {
                if (kv.Value == type)
                {
                    return kv.Key;
                }
            }

            int id = _nextTypeId++;
            Types[id] = type;
            return id;
        }

        private static object ReadValue(IntPtr luaState, int luaIndex, Type type)
        {
            if (type.IsEnum)
            {
                if (ValueTypeMarshaling.TryReadEnumValue(luaState, luaIndex, type, out object enumValue))
                {
                    return enumValue;
                }

                throw new NotSupportedException($"unsupported enum value for {type.Name}");
            }

            if (ValueTypeMarshaling.IsStructType(type))
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object structValue)
                    && structValue != null
                    && structValue.GetType() == type)
                {
                    return structValue;
                }

                throw new NotSupportedException($"unsupported struct value for {type.Name}");
            }

            if (type == typeof(int))
            {
                return (int)LuaDll.lua_tointeger(luaState, luaIndex);
            }
            if (type == typeof(long))
            {
                return LuaDll.lua_tointeger(luaState, luaIndex);
            }
            if (type == typeof(float))
            {
                return (float)LuaDll.lua_tonumber(luaState, luaIndex);
            }
            if (type == typeof(double))
            {
                return LuaDll.lua_tonumber(luaState, luaIndex);
            }
            if (type == typeof(bool))
            {
                return LuaDll.lua_toboolean(luaState, luaIndex) != 0;
            }
            if (type == typeof(string))
            {
                return LuaDllExtension.tostring(luaState, luaIndex);
            }
            throw new NotSupportedException($"unsupported arg type {type.Name}");
        }

        private static int PushReturn(IntPtr luaState, Type returnType, object ret)
        {
            return TypeMethodRegistration.PushReturnValue(luaState, returnType, ret);
        }

        private static bool TryGetUserDataTarget(IntPtr luaState, int index, out object target)
        {
            return ValueTypeMarshaling.TryGetBoxedTarget(luaState, index, out target);
        }

        private static string NormalizeAssemblyName(string assemblyName)
        {
            if (string.IsNullOrEmpty(assemblyName))
            {
                return string.Empty;
            }

            if (assemblyName.EndsWith(".dll", StringComparison.OrdinalIgnoreCase))
            {
                return assemblyName.Substring(0, assemblyName.Length - 4);
            }

            return assemblyName;
        }

        internal static string GetLuaTypeFullName(Type type)
        {
            if (type == null)
            {
                return string.Empty;
            }

            if (type.IsArray)
            {
                Type elementType = type.GetElementType();
                string elementName = elementType != null ? GetLuaTypeFullName(elementType) : type.Name;
                if (type.GetArrayRank() == 1)
                {
                    return elementName + "[]";
                }

                return elementName + "[" + new string(',', type.GetArrayRank() - 1) + "]";
            }

            if (type.IsGenericType && !type.IsGenericTypeDefinition)
            {
                Type genericDef = type.GetGenericTypeDefinition();
                string baseName = GetLuaTypeFullName(genericDef);
                Type[] genericArgs = type.GetGenericArguments();
                if (genericArgs.Length == 0)
                {
                    return baseName;
                }

                var sb = new StringBuilder(baseName.Length + genericArgs.Length * 24);
                sb.Append(baseName).Append('[');
                for (int i = 0; i < genericArgs.Length; i++)
                {
                    if (i > 0)
                    {
                        sb.Append(',');
                    }

                    sb.Append(GetLuaTypeFullName(genericArgs[i]));
                }

                sb.Append(']');
                return sb.ToString();
            }

            if (type.IsNested)
            {
                return GetLuaTypeFullName(type.DeclaringType) + "+" + type.Name;
            }

            if (!string.IsNullOrEmpty(type.Namespace))
            {
                return type.Namespace + "." + type.Name;
            }

            return type.Name;
        }

        private static void PushInternedTypeTable(IntPtr luaState, Type type)
        {
            if (TypeTableRefs.TryGetValue(type, out int tableRef))
            {
                LuaDll.lua_rawgeti(luaState, LuaConsts.LuaRegistryIndex, tableRef);
                return;
            }

            PushTypeTable(luaState, type);
            LuaDll.lua_pushvalue(luaState, -1);
            int newRef = LuaDll.luaL_ref(luaState, LuaConsts.LuaRegistryIndex);
            TypeTableRefs[type] = newRef;
        }

        private static bool TryResolveTypeArg(IntPtr luaState, int index, out Type type)
        {
            type = null;
            LuaDataType luaType = LuaDll.lua_type(luaState, index);
            if (luaType == LuaDataType.String)
            {
                string typeName = LuaDllExtension.tostring(luaState, index);
                if (string.IsNullOrWhiteSpace(typeName))
                {
                    return false;
                }

                type = Mscorlib.GetType(typeName, false);
                return type != null && type.Assembly == Mscorlib;
            }

            if (luaType == LuaDataType.Table)
            {
                return TryResolveTypeFromTypeTable(luaState, index, out type);
            }

            return false;
        }

        private static bool TryResolveTypeFromTypeTable(IntPtr luaState, int index, out Type type)
        {
            type = null;
            LuaDataType idType = RawGetField(luaState, index, "__typeid");
            if (idType == LuaDataType.Number)
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, -1);
                LuaDll.lua_pop(luaState, 1);
                if (Types.TryGetValue(typeId, out type))
                {
                    return true;
                }
            }
            else
            {
                LuaDll.lua_pop(luaState, 1);
            }

            string typeName = ReadTypeNameFromTypeTable(luaState, index);
            string assemblyName = ReadAssemblyNameFromTypeTable(luaState, index);
            if (string.IsNullOrWhiteSpace(typeName) || string.IsNullOrWhiteSpace(assemblyName))
            {
                return false;
            }

            Assembly assembly = ResolveAssembly(NormalizeAssemblyName(assemblyName));
            if (assembly == null)
            {
                return false;
            }

            return TryResolveType(assembly, typeName, out type);
        }

        private static int PushEnumFieldValue(IntPtr luaState, FieldInfo field)
        {
            Type underlying = Enum.GetUnderlyingType(field.FieldType);
            ValueTypeMarshaling.PushUnderlyingInteger(luaState, underlying, field.GetRawConstantValue());
            return 1;
        }
    }
}
