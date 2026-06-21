using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

namespace NovaLua
{
    public sealed class LuaManagerObject
    {
        private static readonly Dictionary<int, MethodInfo> StaticMethods = new Dictionary<int, MethodInfo>();
        private static readonly Dictionary<int, MethodInfo> InstanceMethods = new Dictionary<int, MethodInfo>();
        private static readonly Dictionary<int, Assembly> Assemblies = new Dictionary<int, Assembly>();
        private static readonly Dictionary<int, Type> Types = new Dictionary<int, Type>();
        private static readonly Dictionary<string, int> AssemblyIds = new Dictionary<string, int>(StringComparer.Ordinal);
        private static readonly Dictionary<string, Assembly> AssemblyByLuaName = new Dictionary<string, Assembly>(StringComparer.Ordinal);
        private static readonly Dictionary<string, Type> TypeCache = new Dictionary<string, Type>(StringComparer.Ordinal);

        private static readonly List<LuaCSFunction> CallbackRefs = new List<LuaCSFunction>();
        private static readonly LuaCSFunction CSharpIndexCallback = ResolveAssemblyIndex;
        private static readonly LuaCSFunction AssemblyTypeIndexCallback = ResolveAssemblyTypeIndex;
        private static readonly LuaCSFunction NovaLuaTypeOfCallback = NovaLuaTypeOf;
        private static readonly LuaCSFunction NovaLuaCreateSignatureCallback = NovaLuaCreateSignature;
        private static readonly LuaCSFunction NovaLuaMakeGenericTypeCallback = NovaLuaMakeGenericType;
        private static readonly LuaCSFunction InstanceIndexCallback = InstanceIndex;
        private static readonly LuaCSFunction InstanceNewIndexCallback = InstanceNewIndex;
        private static readonly LuaCSFunction StaticTypeIndexCallback = StaticTypeIndex;
        private static readonly LuaCSFunction StaticTypeNewIndexCallback = StaticTypeNewIndex;

        private static int _nextMethodId = 1;
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

        public void RegisterNovaLuaApi()
        {
            IntPtr luaState = _luaEnv.LuaState;
            LuaDllExtension.RegisterCallback(luaState, "__novalua_typeof", NovaLuaTypeOfCallback);
            LuaDllExtension.RegisterCallback(luaState, "__novalua_create_signature", NovaLuaCreateSignatureCallback);
            LuaDllExtension.RegisterCallback(luaState, "__novalua_make_generic_type", NovaLuaMakeGenericTypeCallback);
        }

        public void RegisterType(Type type)
        {
            IntPtr luaState = _luaEnv.LuaState;
            int oldTop = LuaDll.lua_gettop(luaState);
            try
            {
                EnsureCSharpRoot(luaState); // stack top: CSharp table
                PushAssemblyTable(luaState, type.Assembly); // stack top: assembly table
                PushTypeTable(luaState, type);
                LuaDll.lua_setfield(luaState, -2, type.Name); // assembly[type.Name] = typeTable
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
            int typeId = GetOrCreateTypeId(type);
            LuaDll.lua_createtable(luaState, 0, 16);
            int typeTableIndex = LuaDll.lua_absindex(luaState, -1);

            LuaDll.lua_pushstring(luaState, type.Assembly.GetName().Name);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__assembly");
            LuaDll.lua_pushstring(luaState, type.FullName ?? type.Name);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__fullname");
            LuaDll.lua_pushstring(luaState, type.Name);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__name");

            foreach (MethodInfo method in type.GetMethods(BindingFlags.Public | BindingFlags.Static))
            {
                RegisterStaticMethod(luaState, method);
                SetMethodWithOverloadKeys(luaState, typeTableIndex, method);
            }

            PushInstanceMetatable(luaState, type);
            LuaDll.lua_setfield(luaState, typeTableIndex, "__instance_mt");

            LuaDll.lua_createtable(luaState, 0, 3);
            LuaCSFunction ctorCb = CreateTypeInstance;
            CallbackRefs.Add(ctorCb);
            IntPtr ctorFn = Marshal.GetFunctionPointerForDelegate(ctorCb);
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_pushcclosure(luaState, ctorFn, 1);
            LuaDll.lua_setfield(luaState, -2, "__call");

            CallbackRefs.Add(StaticTypeIndexCallback);
            IntPtr staticIndexFn = Marshal.GetFunctionPointerForDelegate(StaticTypeIndexCallback);
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_pushcclosure(luaState, staticIndexFn, 1);
            LuaDll.lua_setfield(luaState, -2, "__index");

            CallbackRefs.Add(StaticTypeNewIndexCallback);
            IntPtr staticNewIndexFn = Marshal.GetFunctionPointerForDelegate(StaticTypeNewIndexCallback);
            LuaDll.lua_pushinteger(luaState, typeId);
            LuaDll.lua_pushcclosure(luaState, staticNewIndexFn, 1);
            LuaDll.lua_setfield(luaState, -2, "__newindex");

            LuaDll.lua_setmetatable(luaState, typeTableIndex);

            // 清理注册过程中的临时栈项，保证栈顶是 type table 本身。
            LuaDll.lua_settop(luaState, typeTableIndex);
        }

        private static void RegisterStaticMethod(IntPtr luaState, MethodInfo method)
        {
            int methodId = _nextMethodId++;
            StaticMethods[methodId] = method;

            LuaCSFunction cb = InvokeStaticMethod;
            CallbackRefs.Add(cb);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(cb);
            LuaDll.lua_pushinteger(luaState, methodId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        private static void RegisterInstanceMethod(IntPtr luaState, MethodInfo method)
        {
            int methodId = _nextMethodId++;
            InstanceMethods[methodId] = method;

            LuaCSFunction cb = InvokeInstanceMethod;
            CallbackRefs.Add(cb);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(cb);
            LuaDll.lua_pushinteger(luaState, methodId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        private static void PushInstanceMetatable(IntPtr luaState, Type type)
        {
            LuaDll.lua_createtable(luaState, 0, 16);
            int mtIndex = LuaDll.lua_absindex(luaState, -1);

            MethodInfo[] methods = type
                .GetMethods(BindingFlags.Public | BindingFlags.Instance)
                .Where(m => !m.IsSpecialName)
                .ToArray();

            for (int i = 0; i < methods.Length; i++)
            {
                RegisterInstanceMethod(luaState, methods[i]);
                SetMethodWithOverloadKeys(luaState, mtIndex, methods[i]);
            }

            int instanceTypeId = GetOrCreateTypeId(type);

            CallbackRefs.Add(InstanceIndexCallback);
            IntPtr indexFn = Marshal.GetFunctionPointerForDelegate(InstanceIndexCallback);
            LuaDll.lua_pushinteger(luaState, instanceTypeId);
            LuaDll.lua_pushcclosure(luaState, indexFn, 1);
            LuaDll.lua_setfield(luaState, mtIndex, "__index");

            CallbackRefs.Add(InstanceNewIndexCallback);
            IntPtr newIndexFn = Marshal.GetFunctionPointerForDelegate(InstanceNewIndexCallback);
            LuaDll.lua_pushinteger(luaState, instanceTypeId);
            LuaDll.lua_pushcclosure(luaState, newIndexFn, 1);
            LuaDll.lua_setfield(luaState, mtIndex, "__newindex");

            LuaCSFunction gcCb = ReleaseUserData;
            CallbackRefs.Add(gcCb);
            IntPtr gcFn = Marshal.GetFunctionPointerForDelegate(gcCb);
            LuaDll.lua_pushcfunction(luaState, gcFn);
            LuaDll.lua_setfield(luaState, mtIndex, "__gc");
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
                LuaDll.lua_setfield(luaState, 1, luaAssemblyName);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua ResolveAssemblyIndex error: {ex}");
            }
        }

        private static void SetMethodWithOverloadKeys(IntPtr luaState, int tableIndex, MethodInfo method)
        {
            int absIndex = LuaDll.lua_absindex(luaState, tableIndex);

            // 默认名字仅在首次不存在时写入，避免重载互相覆盖。
            LuaDataType existsByName = RawGetField(luaState, absIndex, method.Name);
            if (existsByName == LuaDataType.Nil)
            {
                LuaDll.lua_pop(luaState, 1);
                LuaDll.lua_pushvalue(luaState, -1);
                LuaDll.lua_setfield(luaState, absIndex, method.Name);
            }
            else
            {
                LuaDll.lua_pop(luaState, 1);
            }

            string signature = BuildMethodSignature(method.Name, method.GetParameters().Select(p => p.ParameterType.FullName).ToArray());
            LuaDll.lua_pushvalue(luaState, -1);
            LuaDll.lua_setfield(luaState, absIndex, signature);

            // 当前方法对应的 closure 已写入表中，可从栈顶弹出。
            LuaDll.lua_pop(luaState, 1);
        }

        private static LuaDataType RawGetField(IntPtr luaState, int tableIndex, string key)
        {
            int absIndex = LuaDll.lua_absindex(luaState, tableIndex);
            LuaDll.lua_pushstring(luaState, key);
            return LuaDll.lua_rawget(luaState, absIndex);
        }

        private static string BuildMethodSignature(string methodName, string[] parameterTypeNames)
        {
            if (parameterTypeNames == null || parameterTypeNames.Length == 0)
            {
                return methodName + "()";
            }

            return methodName + "(" + string.Join(",", parameterTypeNames) + ")";
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

                PushTypeTable(luaState, type);
                LuaDll.lua_pushvalue(luaState, -1);
                LuaDll.lua_setfield(luaState, 1, typeName); // cache in assembly table
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua ResolveAssemblyTypeIndex error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int NovaLuaTypeOf(IntPtr luaState)
        {
            try
            {
                // editor 原型阶段：type table 本身即可作为“类型对象”在 novalua helper 中继续使用
                if (LuaDll.lua_type(luaState, 1) == LuaDataType.Table)
                {
                    LuaDll.lua_pushvalue(luaState, 1);
                    return 1;
                }

                return LuaDllExtension.error(luaState, "novalua.typeof expects a csharp type table");
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua typeof error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int NovaLuaCreateSignature(IntPtr luaState)
        {
            try
            {
                string methodName = LuaDllExtension.tostring(luaState, 1);
                if (string.IsNullOrWhiteSpace(methodName))
                {
                    return LuaDllExtension.error(luaState, "novalua.create_signature expects method name");
                }

                int top = LuaDll.lua_gettop(luaState);
                List<string> parameterNames = new List<string>();
                for (int i = 2; i <= top; i++)
                {
                    string typeName = ReadTypeNameFromTypeTable(luaState, i);
                    if (string.IsNullOrWhiteSpace(typeName))
                    {
                        return LuaDllExtension.error(luaState, $"novalua.create_signature arg{i - 1} is not a type");
                    }
                    parameterNames.Add(typeName);
                }

                string signature = BuildMethodSignature(methodName, parameterNames.ToArray());
                LuaDll.lua_pushstring(luaState, signature);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua create_signature error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int NovaLuaMakeGenericType(IntPtr luaState)
        {
            try
            {
                string genericTypeName = ReadTypeNameFromTypeTable(luaState, 1);
                string genericAssemblyName = ReadAssemblyNameFromTypeTable(luaState, 1);
                if (string.IsNullOrWhiteSpace(genericTypeName) || string.IsNullOrWhiteSpace(genericAssemblyName))
                {
                    return LuaDllExtension.error(luaState, "novalua.make_generic_type expects generic type table as first arg");
                }

                Assembly assembly = ResolveAssembly(NormalizeAssemblyName(genericAssemblyName));
                if (assembly == null)
                {
                    return LuaDllExtension.error(luaState, $"assembly not found: {genericAssemblyName}");
                }

                Type genericType = assembly.GetType(genericTypeName, false);
                if (genericType == null)
                {
                    return LuaDllExtension.error(luaState, $"generic type not found: {genericTypeName}");
                }

                int top = LuaDll.lua_gettop(luaState);
                Type[] genericArgs = new Type[Math.Max(top - 1, 0)];
                for (int i = 2; i <= top; i++)
                {
                    string argTypeName = ReadTypeNameFromTypeTable(luaState, i);
                    string argAssemblyName = ReadAssemblyNameFromTypeTable(luaState, i);
                    if (string.IsNullOrWhiteSpace(argTypeName) || string.IsNullOrWhiteSpace(argAssemblyName))
                    {
                        return LuaDllExtension.error(luaState, $"generic arg {i - 1} is not a type");
                    }

                    Assembly argAssembly = ResolveAssembly(NormalizeAssemblyName(argAssemblyName));
                    if (argAssembly == null)
                    {
                        return LuaDllExtension.error(luaState, $"assembly not found: {argAssemblyName}");
                    }

                    Type argType = argAssembly.GetType(argTypeName, false);
                    if (argType == null)
                    {
                        return LuaDllExtension.error(luaState, $"type not found: {argTypeName}");
                    }

                    genericArgs[i - 2] = argType;
                }

                Type closedType = genericType.MakeGenericType(genericArgs);
                PushTypeTable(luaState, closedType);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua make_generic_type error: {ex}");
            }
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
        private static int CreateTypeInstance(IntPtr luaState)
        {
            try
            {
                int upvalueIndex = LuaConsts.LuaRegistryIndex - 1;
                int typeId = (int)LuaDll.lua_tointeger(luaState, upvalueIndex);
                if (!Types.TryGetValue(typeId, out Type type))
                {
                    return LuaDllExtension.error(luaState, $"novalua: type id {typeId} not found");
                }

                int argCount = LuaDll.lua_gettop(luaState) - 1;
                ConstructorInfo ctor = SelectConstructor(type, argCount);
                if (ctor == null)
                {
                    return LuaDllExtension.error(luaState, $"novalua: constructor not found for {type.Name} with {argCount} args");
                }

                object[] args;
                try
                {
                    args = ReadArguments(luaState, ctor.GetParameters(), 2);
                }
                catch (Exception ex)
                {
                    return LuaDllExtension.error(luaState, $"novalua: ctor arg error: {ex.Message}");
                }

                object instance = ctor.Invoke(args);
                GCHandle handle = GCHandle.Alloc(instance);
                IntPtr handlePtr = GCHandle.ToIntPtr(handle);
                IntPtr userData = LuaDll.lua_newuserdatauv(luaState, (UIntPtr)IntPtr.Size, 0);
                Marshal.WriteIntPtr(userData, handlePtr);

                RawGetField(luaState, 1, "__instance_mt");
                LuaDataType mtType = LuaDll.lua_type(luaState, -1);
                if (mtType != LuaDataType.Table)
                {
                    handle.Free();
                    Marshal.WriteIntPtr(userData, IntPtr.Zero);
                    LuaDll.lua_pop(luaState, 1);
                    return LuaDllExtension.error(luaState, $"novalua: instance metatable missing for {type.Name}");
                }

                LuaDll.lua_setmetatable(luaState, -2);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua CreateTypeInstance error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InstanceIndex(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type type))
                {
                    return 0;
                }

                string key = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrEmpty(key))
                {
                    return 0;
                }

                if (LuaDll.lua_getmetatable(luaState, 1) != 0)
                {
                    LuaDataType methodType = RawGetField(luaState, -1, key);
                    if (methodType != LuaDataType.Nil)
                    {
                        // stack: userdata, key, metatable, method
                        LuaDll.lua_remove(luaState, -2);
                        return 1;
                    }
                    LuaDll.lua_pop(luaState, 2);
                }

                if (!TryGetUserDataTarget(luaState, 1, out object target))
                {
                    return 0;
                }

                FieldInfo field = type.GetField(key, BindingFlags.Public | BindingFlags.Instance);
                if (field == null)
                {
                    return 0;
                }

                return PushReturn(luaState, field.FieldType, field.GetValue(target));
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua InstanceIndex error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InstanceNewIndex(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type type))
                {
                    return LuaDllExtension.error(luaState, $"novalua: type id {typeId} not found");
                }

                string key = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrEmpty(key))
                {
                    return LuaDllExtension.error(luaState, "novalua: invalid field name");
                }

                if (!TryGetUserDataTarget(luaState, 1, out object target))
                {
                    return LuaDllExtension.error(luaState, "novalua: invalid userdata for field assignment");
                }

                FieldInfo field = type.GetField(key, BindingFlags.Public | BindingFlags.Instance);
                if (field == null)
                {
                    return LuaDllExtension.error(luaState, $"novalua: instance field not found: {type.Name}.{key}");
                }

                object value;
                try
                {
                    value = ReadValue(luaState, 3, field.FieldType);
                }
                catch (Exception ex)
                {
                    return LuaDllExtension.error(luaState, $"novalua: field arg error: {ex.Message}");
                }

                field.SetValue(target, value);
                return 0;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua InstanceNewIndex error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int StaticTypeIndex(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type type))
                {
                    return 0;
                }

                string key = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrEmpty(key))
                {
                    return 0;
                }

                FieldInfo field = type.GetField(key, BindingFlags.Public | BindingFlags.Static);
                if (field != null)
                {
                    return PushReturn(luaState, field.FieldType, field.GetValue(null));
                }

                LuaDataType existsType = RawGetField(luaState, 1, key);
                return 1;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua StaticTypeIndex error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int StaticTypeNewIndex(IntPtr luaState)
        {
            try
            {
                int typeId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Types.TryGetValue(typeId, out Type type))
                {
                    return LuaDllExtension.error(luaState, $"novalua: type id {typeId} not found");
                }

                string key = LuaDllExtension.tostring(luaState, 2);
                if (string.IsNullOrEmpty(key))
                {
                    return LuaDllExtension.error(luaState, "novalua: invalid field name");
                }

                FieldInfo field = type.GetField(key, BindingFlags.Public | BindingFlags.Static);
                if (field != null)
                {
                    if (field.IsLiteral)
                    {
                        return LuaDllExtension.error(luaState, $"novalua: cannot assign to const field {type.Name}.{key}");
                    }

                    object value;
                    try
                    {
                        value = ReadValue(luaState, 3, field.FieldType);
                    }
                    catch (Exception ex)
                    {
                        return LuaDllExtension.error(luaState, $"novalua: field arg error: {ex.Message}");
                    }

                    field.SetValue(null, value);
                    return 0;
                }

                LuaDll.lua_pushvalue(luaState, 3);
                LuaDll.lua_setfield(luaState, 1, key);
                return 0;
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua StaticTypeNewIndex error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstanceMethod(IntPtr luaState)
        {
            try
            {
                int upvalueIndex = LuaConsts.LuaRegistryIndex - 1;
                int methodId = (int)LuaDll.lua_tointeger(luaState, upvalueIndex);
                if (!InstanceMethods.TryGetValue(methodId, out MethodInfo method))
                {
                    return LuaDllExtension.error(luaState, $"novalua: instance method id {methodId} not found");
                }

                if (!TryGetUserDataTarget(luaState, 1, out object target))
                {
                    return LuaDllExtension.error(luaState, $"novalua: invalid userdata for instance method {method.Name}");
                }

                object[] args;
                try
                {
                    args = ReadArguments(luaState, method.GetParameters(), 2);
                }
                catch (Exception ex)
                {
                    return LuaDllExtension.error(luaState, $"novalua: method arg error: {ex.Message}");
                }

                object ret = method.Invoke(target, args);
                return PushReturn(luaState, method.ReturnType, ret);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua InvokeInstanceMethod error: {ex}");
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int ReleaseUserData(IntPtr luaState)
        {
            try
            {
                IntPtr userData = LuaDll.lua_touserdata(luaState, 1);
                if (userData == IntPtr.Zero)
                {
                    return 0;
                }

                IntPtr handlePtr = Marshal.ReadIntPtr(userData);
                if (handlePtr != IntPtr.Zero)
                {
                    GCHandle handle = GCHandle.FromIntPtr(handlePtr);
                    if (handle.IsAllocated)
                    {
                        handle.Free();
                    }
                    Marshal.WriteIntPtr(userData, IntPtr.Zero);
                }
                return 0;
            }
            catch
            {
                // gc 回调中避免继续抛错导致 native 崩溃
                return 0;
            }
        }

        private static Assembly ResolveAssembly(string luaAssemblyName)
        {
            if (AssemblyByLuaName.TryGetValue(luaAssemblyName, out Assembly cached))
            {
                return cached;
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

        private static bool TryResolveType(Assembly assembly, string typeName, out Type type)
        {
            string cacheKey = assembly.FullName + "::" + typeName;
            if (TypeCache.TryGetValue(cacheKey, out type))
            {
                return type != null;
            }

            type = assembly.GetType(typeName, false);
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
                if (candidate != null && candidate.Name == typeName)
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

        private static ConstructorInfo SelectConstructor(Type type, int argCount)
        {
            ConstructorInfo[] ctors = type.GetConstructors(BindingFlags.Public | BindingFlags.Instance);
            for (int i = 0; i < ctors.Length; i++)
            {
                if (ctors[i].GetParameters().Length == argCount)
                {
                    return ctors[i];
                }
            }
            return null;
        }

        private static object[] ReadArguments(IntPtr luaState, ParameterInfo[] parameters, int startIndex)
        {
            object[] args = new object[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                args[i] = ReadValue(luaState, startIndex + i, parameters[i].ParameterType);
            }
            return args;
        }

        private static object ReadValue(IntPtr luaState, int luaIndex, Type type)
        {
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
            if (returnType == typeof(void))
            {
                return 0;
            }
            if (ret is int i32)
            {
                LuaDll.lua_pushinteger(luaState, i32);
                return 1;
            }
            if (ret is long i64)
            {
                LuaDll.lua_pushinteger(luaState, i64);
                return 1;
            }
            if (ret is float f)
            {
                LuaDll.lua_pushnumber(luaState, f);
                return 1;
            }
            if (ret is double d)
            {
                LuaDll.lua_pushnumber(luaState, d);
                return 1;
            }
            if (ret is bool b)
            {
                LuaDll.lua_pushboolean(luaState, b ? 1 : 0);
                return 1;
            }
            if (ret is string s)
            {
                LuaDll.lua_pushstring(luaState, s);
                return 1;
            }
            throw new NotSupportedException($"unsupported return type {returnType.Name}");
        }

        private static bool TryGetUserDataTarget(IntPtr luaState, int index, out object target)
        {
            IntPtr userData = LuaDll.lua_touserdata(luaState, index);
            if (userData == IntPtr.Zero)
            {
                target = null;
                return false;
            }

            IntPtr handlePtr = Marshal.ReadIntPtr(userData);
            if (handlePtr == IntPtr.Zero)
            {
                target = null;
                return false;
            }

            GCHandle handle = GCHandle.FromIntPtr(handlePtr);
            if (!handle.IsAllocated)
            {
                target = null;
                return false;
            }
            target = handle.Target;
            return target != null;
        }

        private static string NormalizeAssemblyName(string assemblyName)
        {
            if (string.IsNullOrEmpty(assemblyName))
            {
                return string.Empty;
            }

            return assemblyName;
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticMethod(IntPtr luaState)
        {
            int upvalueIndex = LuaConsts.LuaRegistryIndex - 1;
            int methodId = (int)LuaDll.lua_tointeger(luaState, upvalueIndex);
            if (!StaticMethods.TryGetValue(methodId, out MethodInfo method))
            {
                return LuaDllExtension.error(luaState, $"novalua: static method id {methodId} not found");
            }

            object[] args;
            try
            {
                args = ReadArguments(luaState, method.GetParameters(), 1);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua: static arg error: {ex.Message}");
            }

            object ret = method.Invoke(null, args);
            try
            {
                return PushReturn(luaState, method.ReturnType, ret);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua: static return error: {ex.Message}");
            }
        }
    }
}
