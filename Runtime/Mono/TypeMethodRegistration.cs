using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace ZLua
{
    internal static class TypeMethodRegistration
    {
        private static readonly Dictionary<int, MethodInfo> StaticMethods = new Dictionary<int, MethodInfo>();
        private static readonly Dictionary<int, MethodInfo> InstanceMethods = new Dictionary<int, MethodInfo>();
        private static readonly Dictionary<int, LuaCSFunction> StaticFastInvokers = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> InstanceFastInvokers = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, ConstructorInfo> Constructors = new Dictionary<int, ConstructorInfo>();
        private static readonly Dictionary<int, LuaCSFunction> ConstructorCallFastInvokers = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, DispatchGroup> DispatchGroups = new Dictionary<int, DispatchGroup>();
        private static readonly Dictionary<int, LuaCSFunction> DispatchFastInvokers = new Dictionary<int, LuaCSFunction>();
        private static readonly Dictionary<int, LuaCSFunction> ConstructorCallDispatchFastInvokers = new Dictionary<int, LuaCSFunction>();
        private static readonly List<LuaCSFunction> CallbackRefs = new List<LuaCSFunction>();

        private static int _nextMethodId = 1;
        private static int _nextCtorId = 1;
        private static int _nextDispatchId = 1;

        internal sealed class DispatchGroup
        {
            public bool IsStatic;
            public bool IsConstructor;
            public string MethodName;
            public Type OwnerType;
            public int[] MethodIds;
            public int[] ConstructorIds;
        }

        internal static void RegisterStaticMethods(IntPtr luaState, int metatableIndex, Type type)
        {
            List<MethodInfo> methods = CollectMethods(type, isStatic: true);
            BindMethodGroups(luaState, metatableIndex, type, methods, isStatic: true);
            BindMethodAliases(luaState, metatableIndex, methods, isStatic: true);
        }

        internal static void RegisterInstanceMethods(IntPtr luaState, int metatableIndex, Type type)
        {
            List<MethodInfo> methods = CollectMethods(type, isStatic: false);
            BindMethodGroups(luaState, metatableIndex, type, methods, isStatic: false);
            BindMethodAliases(luaState, metatableIndex, methods, isStatic: false);
        }

        /// <summary>
        /// 在创建类型 Lua 表之前校验别名键；失败时抛 <see cref="InvalidOperationException"/>，
        /// 避免部分注册后 C# 异常污染 Lua 栈。
        /// </summary>
        internal static void EnsureMethodAliasKeysValid(Type ownerType)
        {
            ValidateMethodAliasKeys(ownerType, CollectMethods(ownerType, isStatic: true));
            ValidateMethodAliasKeys(ownerType, CollectMethods(ownerType, isStatic: false));
        }

        internal static void RegisterConstructors(IntPtr luaState, int metatableIndex, Type type)
        {
            ConstructorInfo[] ctors = type
                .GetConstructors(BindingFlags.Public | BindingFlags.Instance)
                .OrderBy(GetConstructorSignatureKey, StringComparer.Ordinal)
                .ToArray();

            if (ctors.Length == 0)
            {
                return;
            }

            if (ctors.Length == 1)
            {
                int ctorId = RegisterConstructor(ctors[0]);
                PushConstructorCallClosureFromId(luaState, ctorId);
                LuaDll.lua_setfield(luaState, metatableIndex, "__call");
                return;
            }

            int[] ctorIds = new int[ctors.Length];
            for (int i = 0; i < ctors.Length; i++)
            {
                ctorIds[i] = RegisterConstructor(ctors[i]);
            }

            DispatchGroup group = new DispatchGroup
            {
                IsStatic = true,
                IsConstructor = true,
                MethodName = ".ctor",
                OwnerType = type,
                ConstructorIds = ctorIds,
            };
            int dispatchId = RegisterDispatchGroup(group);
            PushConstructorCallDispatchClosure(luaState, dispatchId);
            LuaDll.lua_setfield(luaState, metatableIndex, "__call");
        }

        private static void BindMethodGroups(IntPtr luaState, int metatableIndex, Type ownerType, List<MethodInfo> methods, bool isStatic)
        {
            Dictionary<string, List<MethodInfo>> groups = new Dictionary<string, List<MethodInfo>>(StringComparer.Ordinal);
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                if (!groups.TryGetValue(method.Name, out List<MethodInfo> list))
                {
                    list = new List<MethodInfo>();
                    groups[method.Name] = list;
                }

                list.Add(method);
            }

            foreach (KeyValuePair<string, List<MethodInfo>> kv in groups)
            {
                List<MethodInfo> overloads = kv.Value;
                if (overloads.Count == 1)
                {
                    PushMethodClosure(luaState, overloads[0], isStatic);
                }
                else
                {
                    int[] methodIds = new int[overloads.Count];
                    for (int i = 0; i < overloads.Count; i++)
                    {
                        methodIds[i] = RegisterMethod(overloads[i], isStatic);
                    }

                    PushDispatchClosure(luaState, new DispatchGroup
                    {
                        IsStatic = isStatic,
                        IsConstructor = false,
                        MethodName = kv.Key,
                        OwnerType = ownerType,
                        MethodIds = methodIds,
                    });
                }

                LuaDll.lua_setfield(luaState, metatableIndex, kv.Key);
            }
        }

        /// <summary>
        /// 将已通过 <see cref="ValidateMethodAliasKeys"/> 的别名写入元表。
        /// 别名与非别名方法名的冲突检查在该校验中完成，此处不再重复。
        /// </summary>
        private static void BindMethodAliases(IntPtr luaState, int metatableIndex, List<MethodInfo> methods, bool isStatic)
        {
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                LuaAliasAttribute aliasAttr = method.GetCustomAttribute<LuaAliasAttribute>();
                if (aliasAttr == null)
                {
                    continue;
                }

                PushMethodClosure(luaState, method, isStatic);
                LuaDll.lua_setfield(luaState, metatableIndex, aliasAttr.Alias);
            }
        }

        /// <summary>
        /// 注册期校验：别名键与默认方法名（非别名）必须互不相交；别名之间亦须唯一。
        /// </summary>
        private static void ValidateMethodAliasKeys(Type ownerType, List<MethodInfo> methods)
        {
            HashSet<string> methodNames = new HashSet<string>(StringComparer.Ordinal);
            for (int i = 0; i < methods.Count; i++)
            {
                methodNames.Add(methods[i].Name);
            }

            Dictionary<string, MethodInfo> aliasOwners = new Dictionary<string, MethodInfo>(StringComparer.Ordinal);
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                LuaAliasAttribute aliasAttr = method.GetCustomAttribute<LuaAliasAttribute>();
                if (aliasAttr == null)
                {
                    continue;
                }

                string alias = aliasAttr.Alias;
                if (string.IsNullOrWhiteSpace(alias))
                {
                    throw new InvalidOperationException(
                        $"zlua: LuaAlias on {ownerType.FullName}.{method.Name} requires a non-empty alias name");
                }

                if (methodNames.Contains(alias))
                {
                    throw new InvalidOperationException(
                        $"zlua: Lua alias '{alias}' on {ownerType.FullName}.{method.Name} must not duplicate a non-alias method name");
                }

                if (aliasOwners.TryGetValue(alias, out MethodInfo existing))
                {
                    throw new InvalidOperationException(
                        $"zlua: duplicate Lua alias '{alias}' on {ownerType.FullName} ({existing.Name} and {method.Name})");
                }

                aliasOwners[alias] = method;
            }
        }

        private static List<MethodInfo> CollectMethods(Type type, bool isStatic)
        {
            BindingFlags flags = (isStatic ? BindingFlags.Static : BindingFlags.Instance)
                | BindingFlags.Public
                | BindingFlags.DeclaredOnly;

            List<MethodInfo> result = new List<MethodInfo>();
            for (Type current = type; current != null; current = current.BaseType)
            {
                MethodInfo[] methods = current.GetMethods(flags);
                Array.Sort(methods, CompareMethodsBySignature);
                for (int i = 0; i < methods.Length; i++)
                {
                    MethodInfo method = methods[i];
                    if (IsSupportedMethod(method))
                    {
                        result.Add(method);
                    }
                }
            }

            return result;
        }

        private static bool IsSupportedMethod(MethodInfo method)
        {
            if (method == null || !method.IsPublic)
            {
                return false;
            }

            if (method.IsGenericMethodDefinition)
            {
                return false;
            }

                if (method.IsSpecialName)
                {
                    if (method.Name == "Invoke"
                        && method.DeclaringType != null
                        && typeof(Delegate).IsAssignableFrom(method.DeclaringType))
                    {
                        return true;
                    }

                    if (method.Name.StartsWith("add_", StringComparison.Ordinal)
                        || method.Name.StartsWith("remove_", StringComparison.Ordinal))
                    {
                        return false;
                    }

                    if (method.Name.StartsWith("get_", StringComparison.Ordinal)
                        || method.Name.StartsWith("set_", StringComparison.Ordinal))
                    {
                        return method.GetParameters().Length > 0;
                    }

                    return false;
                }

            return true;
        }

        internal static object ReadArgumentValue(IntPtr luaState, int luaIndex, Type type)
        {
            return ReadValue(luaState, luaIndex, type);
        }

        internal static bool CanConvertArgumentValue(IntPtr luaState, int luaIndex, Type parameterType)
        {
            return CanConvertLuaValue(luaState, luaIndex, parameterType);
        }

        internal static int PushReturnValue(IntPtr luaState, Type returnType, object ret)
        {
            return PushReturnDefault(luaState, returnType, ret);
        }

        internal static void PushArgumentValue(IntPtr luaState, object value, Type type)
        {
            Type targetType = Nullable.GetUnderlyingType(type) ?? type;
            if (value == null)
            {
                if (PointerMarshaling.IsPointerLikeType(targetType))
                {
                    LuaDll.lua_pushnil(luaState);
                    return;
                }

                if (targetType.IsValueType && Nullable.GetUnderlyingType(type) == null)
                {
                    throw new NotSupportedException($"cannot push null for value type {targetType.Name}");
                }

                LuaDll.lua_pushnil(luaState);
                return;
            }

            if (targetType.IsEnum)
            {
                ValueTypeMarshaling.PushUnderlyingInteger(luaState, Enum.GetUnderlyingType(targetType), value);
                return;
            }

            if (PointerMarshaling.IsTypedReference(targetType))
            {
                throw new NotSupportedException("unsupported push arg type TypedReference");
            }

            if (PointerMarshaling.IsPointerLikeType(targetType))
            {
                PointerMarshaling.PushPointer(luaState, PointerMarshaling.CoerceToAddress(value));
                return;
            }

            if (PointerMarshaling.IsUnsupportedMarshalType(targetType))
            {
                throw new NotSupportedException($"unsupported push arg type {targetType.FullName}");
            }

            if (ValueTypeMarshaling.IsStructType(targetType))
            {
                if (value.GetType() != targetType)
                {
                    throw new NotSupportedException($"unsupported struct value for {targetType.Name}");
                }

                throw new NotSupportedException($"pushing struct argument {targetType.Name} is not supported in delegate bridge yet");
            }

            if (targetType == typeof(int))
            {
                LuaDll.lua_pushinteger(luaState, (int)value);
                return;
            }

            if (targetType == typeof(long))
            {
                LuaDll.lua_pushinteger(luaState, (long)value);
                return;
            }

            if (targetType == typeof(short))
            {
                LuaDll.lua_pushinteger(luaState, (short)value);
                return;
            }

            if (targetType == typeof(byte))
            {
                LuaDll.lua_pushinteger(luaState, (byte)value);
                return;
            }

            if (targetType == typeof(sbyte))
            {
                LuaDll.lua_pushinteger(luaState, (sbyte)value);
                return;
            }

            if (targetType == typeof(uint))
            {
                LuaDll.lua_pushinteger(luaState, (uint)value);
                return;
            }

            if (targetType == typeof(ulong))
            {
                LuaDll.lua_pushinteger(luaState, (long)(ulong)value);
                return;
            }

            if (targetType == typeof(ushort))
            {
                LuaDll.lua_pushinteger(luaState, (ushort)value);
                return;
            }

            if (targetType == typeof(char))
            {
                LuaDll.lua_pushinteger(luaState, (char)value);
                return;
            }

            if (targetType == typeof(IntPtr))
            {
                LuaDll.lua_pushinteger(luaState, ((IntPtr)value).ToInt64());
                return;
            }

            if (targetType == typeof(UIntPtr))
            {
                LuaDll.lua_pushinteger(luaState, (long)(UIntPtr)value);
                return;
            }

            if (targetType == typeof(float))
            {
                LuaDll.lua_pushnumber(luaState, (float)value);
                return;
            }

            if (targetType == typeof(double))
            {
                LuaDll.lua_pushnumber(luaState, (double)value);
                return;
            }

            if (targetType == typeof(bool))
            {
                LuaDll.lua_pushboolean(luaState, (bool)value ? 1 : 0);
                return;
            }

            if (targetType == typeof(string))
            {
                LuaDll.lua_pushstring(luaState, (string)value);
                return;
            }

            if (targetType == typeof(object))
            {
                PushBoxedObjectValue(luaState, value);
                return;
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                LuaManagerObject.PushConstructorInstance(luaState, value, ResolveInstancePushType(targetType, value));
                return;
            }

            if (ShouldPushAsReferenceUserData(targetType))
            {
                LuaManagerObject.PushConstructorInstance(luaState, value, ResolveInstancePushType(targetType, value));
                return;
            }

            throw new NotSupportedException($"unsupported push arg type {targetType.Name}");
        }

        private static void PushBoxedObjectValue(IntPtr luaState, object value)
        {
            if (value == null)
            {
                LuaDll.lua_pushnil(luaState);
                return;
            }

            if (value is bool b)
            {
                LuaDll.lua_pushboolean(luaState, b ? 1 : 0);
                return;
            }

            if (value is string s)
            {
                LuaDll.lua_pushstring(luaState, s);
                return;
            }

            if (value is int i32)
            {
                LuaDll.lua_pushinteger(luaState, i32);
                return;
            }

            if (value is long i64)
            {
                LuaDll.lua_pushinteger(luaState, i64);
                return;
            }

            if (value is float f)
            {
                LuaDll.lua_pushnumber(luaState, f);
                return;
            }

            if (value is double d)
            {
                LuaDll.lua_pushnumber(luaState, d);
                return;
            }

            Type runtimeType = value.GetType();
            if (runtimeType.IsEnum)
            {
                ValueTypeMarshaling.PushUnderlyingInteger(luaState, Enum.GetUnderlyingType(runtimeType), value);
                return;
            }

            if (ValueTypeMarshaling.IsStructType(runtimeType))
            {
                LuaManagerObject.PushConstructorInstance(luaState, value, runtimeType);
                return;
            }

            if (typeof(Delegate).IsAssignableFrom(runtimeType))
            {
                LuaManagerObject.PushConstructorInstance(luaState, value, runtimeType);
                return;
            }

            if (ShouldPushAsReferenceUserData(runtimeType))
            {
                LuaManagerObject.PushConstructorInstance(luaState, value, runtimeType);
                return;
            }

            throw new NotSupportedException($"unsupported object argument type {runtimeType.Name}");
        }

        private static int CompareMethodsBySignature(MethodInfo a, MethodInfo b)
        {
            return string.CompareOrdinal(GetMethodSignatureKey(a), GetMethodSignatureKey(b));
        }

        private static string GetMethodSignatureKey(MethodBase method)
        {
            ParameterInfo[] parameters = method.GetParameters();
            if (parameters.Length == 0)
            {
                return method.Name + "()";
            }

            var sb = new StringBuilder(method.Name.Length + parameters.Length * 16);
            sb.Append(method.Name).Append('(');
            for (int i = 0; i < parameters.Length; i++)
            {
                if (i > 0)
                {
                    sb.Append(',');
                }

                sb.Append(parameters[i].ParameterType.FullName);
            }

            sb.Append(')');
            return sb.ToString();
        }

        private static string GetConstructorSignatureKey(ConstructorInfo ctor)
        {
            return GetMethodSignatureKey(ctor);
        }

        private static int RegisterMethod(MethodInfo method, bool isStatic)
        {
            LuaMarshalAsResolver.ValidateMethodConfiguration(method);
            int methodId = _nextMethodId++;
            if (isStatic)
            {
                StaticMethods[methodId] = method;
                if (LuaToCSharpMethodBridgeFactory.TryCreate(method, isStatic: true, out LuaCSFunction fastInvoker))
                {
                    StaticFastInvokers[methodId] = fastInvoker;
                }
            }
            else
            {
                InstanceMethods[methodId] = method;
                if (LuaToCSharpMethodBridgeFactory.TryCreate(method, isStatic: false, out LuaCSFunction fastInvoker))
                {
                    InstanceFastInvokers[methodId] = fastInvoker;
                }
            }

            return methodId;
        }

        private static int RegisterConstructor(ConstructorInfo ctor)
        {
            LuaMarshalAsResolver.ValidateMethodConfiguration(ctor);
            int ctorId = _nextCtorId++;
            Constructors[ctorId] = ctor;
            if (LuaToCSharpConstructorBridgeFactory.TryCreate(ctor, argStartIndex: 2, out LuaCSFunction callInvoker))
            {
                ConstructorCallFastInvokers[ctorId] = callInvoker;
                CallbackRefs.Add(callInvoker);
            }

            return ctorId;
        }

        private static void PushMethodClosure(IntPtr luaState, MethodInfo method, bool isStatic)
        {
            int methodId = RegisterMethod(method, isStatic);
            if (TryGetFastInvoker(methodId, isStatic, out LuaCSFunction invoker))
            {
                PushDirectCFunction(luaState, invoker);
                return;
            }

            LuaCSFunction stub = isStatic ? InvokeStaticMethod : InvokeInstanceMethod;
            PushStubClosure(luaState, stub, methodId);
        }

        private static void PushConstructorCallClosureFromId(IntPtr luaState, int ctorId)
        {
            if (ConstructorCallFastInvokers.TryGetValue(ctorId, out LuaCSFunction fastInvoker))
            {
                PushCompiledBridge(luaState, fastInvoker);
                return;
            }

            PushStubClosure(luaState, InvokeConstructorCall, ctorId);
        }

        private static void PushConstructorCallDispatchClosure(IntPtr luaState, int dispatchId)
        {
            if (ConstructorCallDispatchFastInvokers.TryGetValue(dispatchId, out LuaCSFunction fastDispatch))
            {
                PushCompiledBridge(luaState, fastDispatch);
                return;
            }

            PushStubClosure(luaState, DispatchConstructorCall, dispatchId);
        }

        private static int RegisterDispatchGroup(DispatchGroup group)
        {
            int dispatchId = _nextDispatchId++;
            DispatchGroups[dispatchId] = group;
            if (!group.IsConstructor
                && LuaToCSharpOverloadDispatchFactory.TryCreate(
                    group,
                    dispatchId,
                    methodId => group.IsStatic ? StaticMethods[methodId] : InstanceMethods[methodId],
                    methodId =>
                    {
                        if (group.IsStatic)
                        {
                            return StaticFastInvokers.TryGetValue(methodId, out LuaCSFunction staticInvoker)
                                ? staticInvoker
                                : null;
                        }

                        return InstanceFastInvokers.TryGetValue(methodId, out LuaCSFunction instanceInvoker)
                            ? instanceInvoker
                            : null;
                    },
                    out LuaCSFunction fastDispatch))
            {
                DispatchFastInvokers[dispatchId] = fastDispatch;
                CallbackRefs.Add(fastDispatch);
            }
            else if (group.IsConstructor)
            {
                if (LuaToCSharpConstructorOverloadDispatchFactory.TryCreate(
                        group,
                        dispatchId,
                        argStartIndex: 2,
                        ctorId => Constructors[ctorId],
                        ctorId => ConstructorCallFastInvokers.TryGetValue(ctorId, out LuaCSFunction invoker) ? invoker : null,
                        out LuaCSFunction callDispatch))
                {
                    ConstructorCallDispatchFastInvokers[dispatchId] = callDispatch;
                    CallbackRefs.Add(callDispatch);
                }
            }

            return dispatchId;
        }

        internal static void PushCompiledBridge(IntPtr luaState, LuaCSFunction invoker)
        {
            PushDirectCFunction(luaState, invoker);
        }

        internal static bool TryFindMethodByParameterSignature(
            Type ownerType,
            string methodName,
            string parameterSignature,
            bool isStatic,
            out MethodInfo method)
        {
            method = null;
            List<MethodInfo> methods = CollectMethods(ownerType, isStatic);
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo candidate = methods[i];
                if (!string.Equals(candidate.Name, methodName, StringComparison.Ordinal))
                {
                    continue;
                }

                if (!string.Equals(
                        LuaManagerObject.FormatParameterSignature(candidate.GetParameters()),
                        parameterSignature,
                        StringComparison.Ordinal))
                {
                    continue;
                }

                method = candidate;
                return true;
            }

            return false;
        }

        internal static bool TryGetRegisteredMethodId(MethodInfo method, bool isStatic, out int methodId)
        {
            methodId = 0;
            if (method == null)
            {
                return false;
            }

            Dictionary<int, MethodInfo> registry = isStatic ? StaticMethods : InstanceMethods;
            foreach (KeyValuePair<int, MethodInfo> kv in registry)
            {
                if (kv.Value.Equals(method))
                {
                    methodId = kv.Key;
                    return true;
                }
            }

            return false;
        }

        internal static void PushRegisteredMethodClosure(IntPtr luaState, MethodInfo method, bool isStatic)
        {
            if (method == null)
            {
                throw new ArgumentNullException(nameof(method));
            }

            if (!TryGetRegisteredMethodId(method, isStatic, out int methodId))
            {
                methodId = RegisterMethod(method, isStatic);
            }

            if (TryGetFastInvoker(methodId, isStatic, out LuaCSFunction invoker))
            {
                PushDirectCFunction(luaState, invoker);
                return;
            }

            LuaCSFunction stub = isStatic ? InvokeStaticMethod : InvokeInstanceMethod;
            PushStubClosure(luaState, stub, methodId);
        }

        private static void PushDispatchCallback(IntPtr luaState, int dispatchId, LuaCSFunction callback)
        {
            PushStubClosure(luaState, callback, dispatchId);
        }

        private static void PushDispatchClosure(IntPtr luaState, DispatchGroup group)
        {
            int dispatchId = RegisterDispatchGroup(group);
            if (!group.IsConstructor && DispatchFastInvokers.TryGetValue(dispatchId, out LuaCSFunction fastDispatch))
            {
                PushDirectCFunction(luaState, fastDispatch);
                return;
            }

            LuaCSFunction cb = group.IsStatic ? DispatchStaticMethod : DispatchInstanceMethod;
            PushStubClosure(luaState, cb, dispatchId);
        }

        private static bool TryGetFastInvoker(int methodId, bool isStatic, out LuaCSFunction invoker)
        {
            if (isStatic)
            {
                return StaticFastInvokers.TryGetValue(methodId, out invoker);
            }

            return InstanceFastInvokers.TryGetValue(methodId, out invoker);
        }

        private static void PushDirectCFunction(IntPtr luaState, LuaCSFunction invoker)
        {
            CallbackRefs.Add(invoker);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(invoker);
            LuaDll.lua_pushcfunction(luaState, fn);
        }

        private static void PushStubClosure(IntPtr luaState, LuaCSFunction callback, int upvalueId)
        {
            CallbackRefs.Add(callback);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(callback);
            LuaDll.lua_pushinteger(luaState, upvalueId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticMethod(IntPtr luaState)
        {
            try
            {
                int methodId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (StaticFastInvokers.TryGetValue(methodId, out LuaCSFunction fastInvoker))
                {
                    return fastInvoker(luaState);
                }

                if (!StaticMethods.TryGetValue(methodId, out MethodInfo method))
                {
                    LuaCallbackBoundary.Throw($"zlua: static method id {methodId} not found");
                }

                return InvokeMethod(luaState, method, argStartIndex: 1, target: null);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstanceMethod(IntPtr luaState)
        {
            try
            {
                int methodId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (InstanceFastInvokers.TryGetValue(methodId, out LuaCSFunction fastInvoker))
                {
                    return fastInvoker(luaState);
                }

                if (!InstanceMethods.TryGetValue(methodId, out MethodInfo method))
                {
                    LuaCallbackBoundary.Throw($"zlua: instance method id {methodId} not found");
                }

                if (!TryGetUserDataTarget(luaState, 1, out object target))
                {
                    LuaCallbackBoundary.Throw($"zlua: invalid userdata for instance method {method.Name}");
                }

                return InvokeMethod(luaState, method, argStartIndex: 2, target);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DispatchStaticMethod(IntPtr luaState)
        {
            try
            {
                int dispatchId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (DispatchFastInvokers.TryGetValue(dispatchId, out LuaCSFunction fastDispatch))
                {
                    return fastDispatch(luaState);
                }

                if (!DispatchGroups.TryGetValue(dispatchId, out DispatchGroup group))
                {
                    LuaCallbackBoundary.Throw($"zlua: dispatch group {dispatchId} not found");
                }

                return DispatchMethods(luaState, group, argStartIndex: 1, target: null);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DispatchInstanceMethod(IntPtr luaState)
        {
            try
            {
                int dispatchId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (DispatchFastInvokers.TryGetValue(dispatchId, out LuaCSFunction fastDispatch))
                {
                    return fastDispatch(luaState);
                }

                if (!DispatchGroups.TryGetValue(dispatchId, out DispatchGroup group))
                {
                    return LuaDllExtension.error(luaState, $"zlua: dispatch group {dispatchId} not found");
                }

                if (!TryGetUserDataTarget(luaState, 1, out object target))
                {
                    return LuaDllExtension.error(luaState, $"zlua: invalid userdata for method {group.MethodName}");
                }

                return DispatchMethods(luaState, group, argStartIndex: 2, target);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DispatchConstructorCall(IntPtr luaState)
        {
            try
            {
                int dispatchId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!DispatchGroups.TryGetValue(dispatchId, out DispatchGroup group) || !group.IsConstructor)
                {
                    LuaCallbackBoundary.Throw($"zlua: constructor dispatch group {dispatchId} not found");
                }

                const int argStartIndex = 2;
                int[] ctorIds = group.ConstructorIds;
                List<string> candidates = new List<string>(ctorIds.Length);
                for (int i = 0; i < ctorIds.Length; i++)
                {
                    if (!Constructors.TryGetValue(ctorIds[i], out ConstructorInfo ctor))
                    {
                        continue;
                    }

                    candidates.Add(GetMethodSignatureKey(ctor));
                    if (TryBuildArguments(luaState, ctor, argStartIndex, out object[] args))
                    {
                        return InvokePreparedConstructorCall(luaState, ctor, args);
                    }
                }

                LuaCallbackBoundary.Throw(BuildNoOverloadMessage(group.OwnerType, null, luaState, argStartIndex, candidates));
                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeConstructorCall(IntPtr luaState)
        {
            try
            {
                int ctorId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Constructors.TryGetValue(ctorId, out ConstructorInfo ctor))
                {
                    LuaCallbackBoundary.Throw($"zlua: constructor id {ctorId} not found");
                }

                const int argStartIndex = 2;
                if (!TryBuildArguments(luaState, ctor, argStartIndex, out object[] args))
                {
                    LuaCallbackBoundary.Throw(
                        $"zlua: constructor argument mismatch for {ctor.DeclaringType.FullName}{GetParameterSignature(ctor.GetParameters())}");
                }

                return InvokePreparedConstructorCall(luaState, ctor, args);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        private static int DispatchMethods(IntPtr luaState, DispatchGroup group, int argStartIndex, object target)
        {
            int[] methodIds = group.MethodIds;
            Dictionary<int, LuaCSFunction> fastInvokers = group.IsStatic ? StaticFastInvokers : InstanceFastInvokers;
            List<string> candidates = new List<string>(methodIds.Length);
            for (int i = 0; i < methodIds.Length; i++)
            {
                int methodId = methodIds[i];
                MethodInfo method = group.IsStatic
                    ? StaticMethods[methodId]
                    : InstanceMethods[methodId];
                candidates.Add(GetMethodSignatureKey(method));
                if (!TryCanConvertArguments(luaState, method, argStartIndex))
                {
                    continue;
                }

                if (group.IsStatic
                    && fastInvokers.TryGetValue(methodId, out LuaCSFunction fastInvoker))
                {
                    return fastInvoker(luaState);
                }

                if (TryBuildArguments(luaState, method, argStartIndex, out object[] args))
                {
                    return InvokePreparedMethod(luaState, method, target, args);
                }
            }

            return FailNoOverloadForDispatch(luaState, group, argStartIndex, candidates);
        }

        internal static int FailNoOverloadForDispatch(IntPtr luaState, DispatchGroup group, int argStartIndex)
        {
            if (group.IsConstructor)
            {
                int[] ctorIds = group.ConstructorIds;
                var candidates = new List<string>(ctorIds.Length);
                for (int i = 0; i < ctorIds.Length; i++)
                {
                    if (Constructors.TryGetValue(ctorIds[i], out ConstructorInfo ctor))
                    {
                        candidates.Add(GetMethodSignatureKey(ctor));
                    }
                }

                return FailNoOverloadForDispatch(luaState, group, argStartIndex, candidates);
            }

            int[] methodIds = group.MethodIds;
            var methodCandidates = new List<string>(methodIds.Length);
            for (int i = 0; i < methodIds.Length; i++)
            {
                MethodInfo method = group.IsStatic
                    ? StaticMethods[methodIds[i]]
                    : InstanceMethods[methodIds[i]];
                methodCandidates.Add(GetMethodSignatureKey(method));
            }

            return FailNoOverloadForDispatch(luaState, group, argStartIndex, methodCandidates);
        }

        private static int FailNoOverloadForDispatch(
            IntPtr luaState,
            DispatchGroup group,
            int argStartIndex,
            List<string> candidates)
        {
            string methodName = group.IsConstructor ? null : group.MethodName;
            return LuaDllExtension.error(
                luaState,
                BuildNoOverloadMessage(group.OwnerType, methodName, luaState, argStartIndex, candidates));
        }

        private static int InvokeMethod(IntPtr luaState, MethodInfo method, int argStartIndex, object target)
        {
            if (!TryBuildArguments(luaState, method, argStartIndex, out object[] args))
            {
                LuaCallbackBoundary.Throw(
                    $"zlua: argument mismatch for {method.DeclaringType.FullName}.{method.Name}{GetParameterSignature(method.GetParameters())}");
            }

            return InvokePreparedMethod(luaState, method, target, args);
        }

        private static int InvokePreparedMethod(IntPtr luaState, MethodInfo method, object target, object[] args)
        {
            try
            {
                StructOpaqueScope.EnterLuaToCSharp();

                if (target is Array array)
                {
                    ArrayMarshaling.CoerceSetValueArguments(array, method, args);
                }

                object ret;
                if (PointerMethodInvoker.RequiresPointerInvoke(method))
                {
                    ret = PointerMethodInvoker.Invoke(method, target, args);
                }
                else if (PointerMethodInvoker.HasUnsafeReflectionSignature(method))
                {
                    throw new NotSupportedException(
                        $"zlua: method signature cannot be invoked via reflection: {method.DeclaringType.FullName}.{method.Name}{GetParameterSignature(method.GetParameters())}");
                }
                else
                {
                    ret = method.Invoke(target, args);
                }

                return PushReturn(luaState, method, ret);
            }
            finally
            {
                StructOpaqueScope.LeaveLuaToCSharp();
            }
        }

        private static int InvokeConstructorInstance(IntPtr luaState, Type type, ConstructorInfo ctor, object[] args)
        {
            object instance = ctor.Invoke(args);
            return LuaManagerObject.PushConstructorInstance(luaState, instance, type);
        }

        private static int InvokePreparedConstructorCall(IntPtr luaState, ConstructorInfo ctor, object[] args)
        {
            try
            {
                StructOpaqueScope.EnterLuaToCSharp();
                object instance = ctor.Invoke(args);
                return LuaManagerObject.PushConstructorInstance(luaState, instance, ctor.DeclaringType);
            }
            finally
            {
                StructOpaqueScope.LeaveLuaToCSharp();
            }
        }

        internal static bool TryCanConvertArguments(IntPtr luaState, MethodBase method, int argStartIndex)
        {
            ParameterInfo[] parameters = method.GetParameters();
            int luaArgCount = LuaDll.lua_gettop(luaState) - argStartIndex + 1;
            if (luaArgCount < 0)
            {
                luaArgCount = 0;
            }

            int paramsIndex = FindParamsIndex(parameters);
            if (paramsIndex >= 0)
            {
                int fixedCount = paramsIndex;
                if (luaArgCount < fixedCount)
                {
                    return false;
                }

                for (int i = 0; i < fixedCount; i++)
                {
                    if (!CanConvertArgument(luaState, argStartIndex + i, parameters[i], method))
                    {
                        return false;
                    }
                }

                Type paramsType = parameters[paramsIndex].ParameterType;
                int varCount = luaArgCount - fixedCount;
                if (varCount == 1 && paramsType.IsArray)
                {
                    return CanConvertArgument(luaState, argStartIndex + fixedCount, parameters[paramsIndex], method);
                }

                Type elementType = paramsType.GetElementType();
                for (int i = 0; i < varCount; i++)
                {
                    if (!CanConvertArgument(
                            luaState,
                            argStartIndex + fixedCount + i,
                            parameters[paramsIndex],
                            method,
                            elementType))
                    {
                        return false;
                    }
                }

                return true;
            }

            if (luaArgCount > parameters.Length)
            {
                return false;
            }

            for (int i = 0; i < parameters.Length; i++)
            {
                if (i < luaArgCount)
                {
                    if (!CanConvertArgument(luaState, argStartIndex + i, parameters[i], method))
                    {
                        return false;
                    }
                }
                else if (!parameters[i].HasDefaultValue)
                {
                    return false;
                }
            }

            return true;
        }

        internal static bool TryBuildArguments(IntPtr luaState, MethodBase method, int argStartIndex, out object[] args)
        {
            args = null;
            ParameterInfo[] parameters = method.GetParameters();
            int luaArgCount = LuaDll.lua_gettop(luaState) - argStartIndex + 1;
            if (luaArgCount < 0)
            {
                luaArgCount = 0;
            }

            int paramsIndex = FindParamsIndex(parameters);
            if (paramsIndex >= 0)
            {
                int fixedCount = paramsIndex;
                if (luaArgCount < fixedCount)
                {
                    return false;
                }

                args = new object[parameters.Length];
                for (int i = 0; i < fixedCount; i++)
                {
                    if (!TryReadArgument(luaState, argStartIndex + i, parameters[i], method, out object value))
                    {
                        return false;
                    }

                    args[i] = value;
                }

                Type paramsType = parameters[paramsIndex].ParameterType;
                int varCount = luaArgCount - fixedCount;
                if (varCount == 1 && paramsType.IsArray)
                {
                    if (TryReadArgument(luaState, argStartIndex + fixedCount, parameters[paramsIndex], method, out object arrayArg))
                    {
                        args[paramsIndex] = arrayArg;
                        return true;
                    }
                }

                Type elementType = paramsType.GetElementType();
                Array array = Array.CreateInstance(elementType, varCount);
                for (int i = 0; i < varCount; i++)
                {
                    if (!TryReadArgument(
                            luaState,
                            argStartIndex + fixedCount + i,
                            parameters[paramsIndex],
                            method,
                            out object value,
                            elementType))
                    {
                        return false;
                    }

                    array.SetValue(ArrayMarshaling.CoerceToElementType(value, elementType), i);
                }

                args[paramsIndex] = array;
                return true;
            }

            if (luaArgCount > parameters.Length)
            {
                return false;
            }

            args = new object[parameters.Length];
            for (int i = 0; i < parameters.Length; i++)
            {
                if (i < luaArgCount)
                {
                    if (!TryReadArgument(luaState, argStartIndex + i, parameters[i], method, out object value))
                    {
                        return false;
                    }

                    args[i] = value;
                }
                else if (parameters[i].HasDefaultValue)
                {
                    args[i] = parameters[i].DefaultValue;
                }
                else
                {
                    return false;
                }
            }

            return true;
        }

        private static int FindParamsIndex(ParameterInfo[] parameters)
        {
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i].IsDefined(typeof(ParamArrayAttribute), false))
                {
                    return i;
                }
            }

            return -1;
        }

        private static bool CanConvertArgument(
            IntPtr luaState,
            int luaIndex,
            ParameterInfo parameter,
            MethodBase method,
            Type effectiveClrType = null)
        {
            Type parameterType = effectiveClrType ?? parameter.ParameterType;
            LuaMarshalType marshalType = LuaMarshalAsResolver.ResolveParameter(
                parameter,
                method,
                LuaMarshalDirection.LuaToCSharp,
                parameterType);

            if (marshalType != LuaMarshalType.Default)
            {
                return LuaMarshalDispatch.CanConvert(luaState, luaIndex, parameterType, marshalType);
            }

            return CanConvertLuaValue(luaState, luaIndex, parameterType);
        }

        private static bool TryReadArgument(
            IntPtr luaState,
            int luaIndex,
            ParameterInfo parameter,
            MethodBase method,
            out object value,
            Type effectiveClrType = null)
        {
            value = null;
            Type parameterType = effectiveClrType ?? parameter.ParameterType;
            LuaMarshalType marshalType = LuaMarshalAsResolver.ResolveParameter(
                parameter,
                method,
                LuaMarshalDirection.LuaToCSharp,
                parameterType);

            if (marshalType != LuaMarshalType.Default)
            {
                if (!LuaMarshalDispatch.CanConvert(luaState, luaIndex, parameterType, marshalType))
                {
                    return false;
                }

                value = LuaMarshalDispatch.Read(luaState, luaIndex, parameterType, marshalType);
                return true;
            }

            if (!CanConvertLuaValue(luaState, luaIndex, parameterType))
            {
                return false;
            }

            value = ReadValue(luaState, luaIndex, parameterType);
            return true;
        }

        private static bool CanConvertLuaValue(IntPtr luaState, int luaIndex, Type parameterType)
        {
            Type targetType = Nullable.GetUnderlyingType(parameterType) ?? parameterType;
            if (targetType.IsByRef)
            {
                targetType = targetType.GetElementType();
            }

            LuaDataType luaType = LuaDll.lua_type(luaState, luaIndex);
            if (luaType == LuaDataType.Nil)
            {
                return !targetType.IsValueType
                    || Nullable.GetUnderlyingType(parameterType) != null
                    || PointerMarshaling.IsPointerLikeType(targetType);
            }

            if (PointerMarshaling.IsUnsupportedMarshalType(targetType))
            {
                return false;
            }

            if (PointerMarshaling.IsPointerLikeType(targetType))
            {
                return PointerMarshaling.CanConvertPointerValue(luaState, luaIndex);
            }

            if (targetType == typeof(string))
            {
                return luaType == LuaDataType.String;
            }

            if (targetType == typeof(bool))
            {
                return luaType == LuaDataType.Boolean;
            }

            if (targetType == typeof(int) || targetType == typeof(long)
                || targetType == typeof(short) || targetType == typeof(byte)
                || targetType == typeof(uint) || targetType == typeof(ulong)
                || targetType == typeof(ushort) || targetType == typeof(sbyte)
                || targetType == typeof(char))
            {
                return IsLuaInteger(luaState, luaIndex, targetType);
            }

            if (targetType == typeof(IntPtr) || targetType == typeof(UIntPtr))
            {
                return IsLuaInteger(luaState, luaIndex, IntPtr.Size == 8 ? typeof(long) : typeof(int));
            }

            if (targetType == typeof(float) || targetType == typeof(double))
            {
                return luaType == LuaDataType.Number;
            }

            if (targetType.IsEnum)
            {
                if (luaType == LuaDataType.Number)
                {
                    return ValueTypeMarshaling.TryReadUnderlyingInteger(luaState, luaIndex, Enum.GetUnderlyingType(targetType), out _);
                }

                return luaType == LuaDataType.UserData;
            }

            if (ValueTypeMarshaling.IsStructType(targetType))
            {
                if (luaType == LuaDataType.UserData)
                {
                    return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                        && boxed != null
                        && boxed.GetType() == targetType;
                }

                return false;
            }

            if (targetType == typeof(object))
            {
                return luaType == LuaDataType.Boolean
                    || luaType == LuaDataType.Number
                    || luaType == LuaDataType.String
                    || luaType == LuaDataType.UserData;
            }

            if (targetType.IsArray)
            {
                if (luaType != LuaDataType.UserData)
                {
                    return false;
                }

                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed is Array array
                    && targetType.IsAssignableFrom(array.GetType());
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                if (luaType == LuaDataType.Nil)
                {
                    return true;
                }

                if (luaType == LuaDataType.Function)
                {
                    return true;
                }

                if (luaType != LuaDataType.UserData)
                {
                    return false;
                }

                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed is Delegate del
                    && targetType.IsAssignableFrom(del.GetType());
            }

            if (targetType.IsClass || targetType.IsInterface)
            {
                if (luaType != LuaDataType.UserData)
                {
                    return false;
                }

                return ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed != null
                    && targetType.IsAssignableFrom(boxed.GetType());
            }

            return false;
        }

        private static bool IsLuaInteger(IntPtr luaState, int luaIndex, Type targetType)
        {
            if (LuaDll.lua_type(luaState, luaIndex) != LuaDataType.Number)
            {
                return false;
            }

            if (LuaDll.lua_isinteger(luaState, luaIndex) != 0)
            {
                long value = LuaDll.lua_tointeger(luaState, luaIndex);
                return FitsIntegerRange(value, targetType);
            }

            double number = LuaDll.lua_tonumber(luaState, luaIndex);
            if (Math.Truncate(number) != number)
            {
                return false;
            }

            long integral = (long)number;
            return (double)integral == number && FitsIntegerRange(integral, targetType);
        }

        private static bool FitsIntegerRange(long value, Type targetType)
        {
            if (targetType == typeof(long) || targetType == typeof(ulong))
            {
                return true;
            }

            if (targetType == typeof(int))
            {
                return value >= int.MinValue && value <= int.MaxValue;
            }

            if (targetType == typeof(uint))
            {
                return value >= 0 && value <= uint.MaxValue;
            }

            if (targetType == typeof(short))
            {
                return value >= short.MinValue && value <= short.MaxValue;
            }

            if (targetType == typeof(ushort))
            {
                return value >= 0 && value <= ushort.MaxValue;
            }

            if (targetType == typeof(byte))
            {
                return value >= byte.MinValue && value <= byte.MaxValue;
            }

            if (targetType == typeof(sbyte))
            {
                return value >= sbyte.MinValue && value <= sbyte.MaxValue;
            }

            if (targetType == typeof(char))
            {
                return value >= char.MinValue && value <= char.MaxValue;
            }

            return false;
        }

        private static string BuildNoOverloadMessage(Type ownerType, string methodName, IntPtr luaState, int argStartIndex, List<string> candidates)
        {
            int luaArgCount = Math.Max(0, LuaDll.lua_gettop(luaState) - argStartIndex + 1);
            var argTags = new List<string>(luaArgCount);
            for (int i = 0; i < luaArgCount; i++)
            {
                argTags.Add(LuaDll.lua_type(luaState, argStartIndex + i).ToString());
            }

            string target = string.IsNullOrEmpty(methodName)
                ? ownerType.FullName
                : $"{ownerType.FullName}.{methodName}";
            return $"no overload for {target} matching ({string.Join(", ", argTags)}); candidates: {string.Join(", ", candidates)}";
        }

        private static string GetParameterSignature(ParameterInfo[] parameters)
        {
            if (parameters == null || parameters.Length == 0)
            {
                return "()";
            }

            return "(" + string.Join(",", parameters.Select(p => p.ParameterType.FullName)) + ")";
        }

        private static LuaDataType RawGetField(IntPtr luaState, int tableIndex, string key)
        {
            int absIndex = LuaDll.lua_absindex(luaState, tableIndex);
            LuaDll.lua_pushstring(luaState, key);
            return LuaDll.lua_rawget(luaState, absIndex);
        }

        private static object ReadValue(IntPtr luaState, int luaIndex, Type type)
        {
            if (LuaDll.lua_type(luaState, luaIndex) == LuaDataType.Nil)
            {
                Type nilTargetType = Nullable.GetUnderlyingType(type) ?? type;
                if (PointerMarshaling.IsPointerLikeType(nilTargetType))
                {
                    return PointerMarshaling.BoxPointerForInvoke(IntPtr.Zero, nilTargetType);
                }

                return null;
            }

            Type targetType = Nullable.GetUnderlyingType(type) ?? type;
            if (PointerMarshaling.IsTypedReference(targetType))
            {
                throw new NotSupportedException("unsupported arg type TypedReference");
            }

            if (PointerMarshaling.IsPointerLikeType(targetType))
            {
                IntPtr address = PointerMarshaling.ReadPointer(luaState, luaIndex);
                return PointerMarshaling.BoxPointerForInvoke(address, targetType);
            }

            if (PointerMarshaling.IsUnsupportedMarshalType(targetType))
            {
                throw new NotSupportedException($"unsupported arg type {targetType.FullName}");
            }

            if (targetType.IsEnum)
            {
                if (ValueTypeMarshaling.TryReadEnumValue(luaState, luaIndex, targetType, out object enumValue))
                {
                    return enumValue;
                }

                throw new NotSupportedException($"unsupported enum value for {targetType.Name}");
            }

            if (ValueTypeMarshaling.IsStructType(targetType))
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object structValue)
                    && structValue != null
                    && structValue.GetType() == targetType)
                {
                    return structValue;
                }

                throw new NotSupportedException($"unsupported struct value for {targetType.Name}");
            }

            if (targetType == typeof(int))
            {
                return (int)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(long))
            {
                return LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(short))
            {
                return (short)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(byte))
            {
                return (byte)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(sbyte))
            {
                return (sbyte)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(uint))
            {
                return (uint)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(ulong))
            {
                return (ulong)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(ushort))
            {
                return (ushort)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(char))
            {
                return (char)(int)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(IntPtr))
            {
                return new IntPtr(LuaDll.lua_tointeger(luaState, luaIndex));
            }

            if (targetType == typeof(UIntPtr))
            {
                return (UIntPtr)(ulong)LuaDll.lua_tointeger(luaState, luaIndex);
            }

            if (targetType == typeof(float))
            {
                return (float)LuaDll.lua_tonumber(luaState, luaIndex);
            }

            if (targetType == typeof(double))
            {
                return LuaDll.lua_tonumber(luaState, luaIndex);
            }

            if (targetType == typeof(bool))
            {
                return LuaDll.lua_toboolean(luaState, luaIndex) != 0;
            }

            if (targetType == typeof(string))
            {
                return LuaDllExtension.tostring(luaState, luaIndex);
            }

            if (targetType == typeof(object))
            {
                return ReadBoxedObjectValue(luaState, luaIndex);
            }

            if (targetType.IsArray)
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object arrayValue)
                    && arrayValue is Array array
                    && targetType.IsAssignableFrom(array.GetType()))
                {
                    return array;
                }

                throw new NotSupportedException($"unsupported array value for {targetType.Name}");
            }

            if (typeof(Delegate).IsAssignableFrom(targetType))
            {
                return ReadDelegateValue(luaState, luaIndex, targetType);
            }

            if (targetType.IsClass || targetType.IsInterface)
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object referenceValue)
                    && referenceValue != null
                    && targetType.IsAssignableFrom(referenceValue.GetType()))
                {
                    return referenceValue;
                }

                throw new NotSupportedException($"unsupported reference value for {targetType.Name}");
            }

            throw new NotSupportedException($"unsupported arg type {targetType.Name}");
        }

        private static object ReadDelegateValue(IntPtr luaState, int luaIndex, Type delegateType)
        {
            LuaDataType luaType = LuaDll.lua_type(luaState, luaIndex);
            if (luaType == LuaDataType.Nil)
            {
                return null;
            }

            if (luaType == LuaDataType.UserData)
            {
                if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed)
                    && boxed is Delegate del
                    && delegateType.IsAssignableFrom(del.GetType()))
                {
                    return del;
                }

                throw new NotSupportedException($"unsupported delegate userdata for {delegateType.Name}");
            }

            if (luaType != LuaDataType.Function)
            {
                throw new NotSupportedException($"unsupported Lua value for delegate {delegateType.Name}");
            }

            int funcRef = LuaDelegateBinder.CreateFunctionRef(luaState, luaIndex);
            try
            {
                return LuaDelegateBinder.Create(LuaMonoAppDomain.LuaEnv, delegateType, funcRef);
            }
            catch
            {
                LuaDll.luaL_unref(luaState, LuaConsts.LuaRegistryIndex, funcRef);
                throw;
            }
        }

        private static object ReadBoxedObjectValue(IntPtr luaState, int luaIndex)
        {
            switch (LuaDll.lua_type(luaState, luaIndex))
            {
                case LuaDataType.Boolean:
                    return LuaDll.lua_toboolean(luaState, luaIndex) != 0;
                case LuaDataType.Number:
                    if (LuaDll.lua_isinteger(luaState, luaIndex) != 0)
                    {
                        long integral = LuaDll.lua_tointeger(luaState, luaIndex);
                        if (integral >= int.MinValue && integral <= int.MaxValue)
                        {
                            return (int)integral;
                        }

                        return integral;
                    }

                    return LuaDll.lua_tonumber(luaState, luaIndex);
                case LuaDataType.String:
                    return LuaDllExtension.tostring(luaState, luaIndex);
                case LuaDataType.UserData:
                    if (ValueTypeMarshaling.TryGetBoxedTarget(luaState, luaIndex, out object boxed))
                    {
                        return boxed;
                    }

                    throw new NotSupportedException("unsupported userdata for object");
                default:
                    throw new NotSupportedException("unsupported Lua value for object");
            }
        }

        private static Type ResolveInstancePushType(Type declaredType, object instance)
        {
            // interface / base-class returns must push userdata by runtime implementation type (MARSHAL_SPEC §1).
            return instance?.GetType() ?? declaredType;
        }

        private static bool ShouldPushAsReferenceUserData(Type type)
        {
            if (type == typeof(string) || type == typeof(object))
            {
                return false;
            }

            return type.IsArray || type.IsInterface || type.IsClass;
        }

        private static bool IsNullNullableReturn(Type returnType, object ret)
        {
            if (Nullable.GetUnderlyingType(returnType) == null)
            {
                return ret == null;
            }

            if (ret == null)
            {
                return true;
            }

            Type runtimeType = ret.GetType();
            if (runtimeType.IsGenericType && runtimeType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                PropertyInfo hasValueProperty = runtimeType.GetProperty("HasValue", BindingFlags.Public | BindingFlags.Instance);
                if (hasValueProperty?.GetValue(ret) is bool hasValue)
                {
                    return !hasValue;
                }
            }

            return false;
        }

        private static object UnwrapNullableValue(object ret)
        {
            if (ret == null)
            {
                return null;
            }

            Type runtimeType = ret.GetType();
            if (runtimeType.IsGenericType && runtimeType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                return runtimeType.GetProperty("Value", BindingFlags.Public | BindingFlags.Instance)?.GetValue(ret);
            }

            return ret;
        }

        private static int PushReturn(IntPtr luaState, MethodInfo method, object ret)
        {
            Type returnType = method.ReturnType;
            if (returnType == typeof(void))
            {
                return 0;
            }

            LuaMarshalType marshalType = LuaMarshalAsResolver.ResolveReturn(method, LuaMarshalDirection.CSharpToLua);
            if (marshalType != LuaMarshalType.Default)
            {
                return LuaMarshalDispatch.PushReturn(luaState, returnType, ret, marshalType);
            }

            return PushReturnDefault(luaState, returnType, ret);
        }

        private static int PushReturnDefault(IntPtr luaState, Type returnType, object ret)
        {
            if (returnType == typeof(void))
            {
                return 0;
            }

            Type unwrappedReturnType = Nullable.GetUnderlyingType(returnType) ?? returnType;
            if (unwrappedReturnType.IsEnum)
            {
                if (IsNullNullableReturn(returnType, ret))
                {
                    LuaDll.lua_pushnil(luaState);
                    return 1;
                }

                if (ret == null)
                {
                    throw new NotSupportedException($"unsupported null return for enum type {returnType.Name}");
                }

                object enumValue = UnwrapNullableValue(ret);
                ValueTypeMarshaling.PushUnderlyingInteger(luaState, Enum.GetUnderlyingType(unwrappedReturnType), enumValue);
                return 1;
            }

            if (PointerMarshaling.IsTypedReference(unwrappedReturnType))
            {
                throw new NotSupportedException("unsupported return type TypedReference");
            }

            if (PointerMarshaling.IsPointerLikeType(unwrappedReturnType))
            {
                IntPtr address = ret == null ? IntPtr.Zero : PointerMarshaling.CoerceToAddress(ret);
                PointerMarshaling.PushPointer(luaState, address);
                return 1;
            }

            if (ValueTypeMarshaling.IsStructType(unwrappedReturnType))
            {
                if (IsNullNullableReturn(returnType, ret))
                {
                    LuaDll.lua_pushnil(luaState);
                    return 1;
                }

                return LuaManagerObject.PushConstructorInstance(luaState, ret, unwrappedReturnType);
            }

            if (ret == null)
            {
                if (unwrappedReturnType.IsValueType && Nullable.GetUnderlyingType(returnType) == null)
                {
                    throw new NotSupportedException($"unsupported null return for value type {returnType.Name}");
                }

                LuaDll.lua_pushnil(luaState);
                return 1;
            }

            if (typeof(Delegate).IsAssignableFrom(unwrappedReturnType))
            {
                return LuaManagerObject.PushConstructorInstance(luaState, ret, ResolveInstancePushType(unwrappedReturnType, ret));
            }

            if (ShouldPushAsReferenceUserData(unwrappedReturnType))
            {
                return LuaManagerObject.PushConstructorInstance(luaState, ret, ResolveInstancePushType(unwrappedReturnType, ret));
            }

            PushArgumentValue(luaState, ret, unwrappedReturnType);
            return 1;
        }

        private static bool TryGetUserDataTarget(IntPtr luaState, int index, out object target)
        {
            return ValueTypeMarshaling.TryGetBoxedTarget(luaState, index, out target);
        }

        internal static void EnsureConvertible(IntPtr luaState, int index, Type parameterType)
        {
            if (!CanConvertLuaValue(luaState, index, parameterType))
            {
                LuaCallbackBoundary.Throw(
                    $"zlua: argument mismatch at index {index} for type {parameterType.FullName}");
            }
        }

        internal static object ReadValuePublic(IntPtr luaState, int index, Type type)
        {
            return ReadValue(luaState, index, type);
        }

        internal static int PushReturnDefaultPublic(IntPtr luaState, Type returnType, object ret)
        {
            return PushReturnDefault(luaState, returnType, ret);
        }

        internal static string GetParameterSignaturePublic(ParameterInfo[] parameters)
        {
            return GetParameterSignature(parameters);
        }
    }
}
