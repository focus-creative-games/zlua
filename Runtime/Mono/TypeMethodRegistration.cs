using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace NovaLua
{
    internal static class TypeMethodRegistration
    {
        private static readonly Dictionary<int, MethodInfo> StaticMethods = new Dictionary<int, MethodInfo>();
        private static readonly Dictionary<int, MethodInfo> InstanceMethods = new Dictionary<int, MethodInfo>();
        private static readonly Dictionary<int, ConstructorInfo> Constructors = new Dictionary<int, ConstructorInfo>();
        private static readonly Dictionary<int, DispatchGroup> DispatchGroups = new Dictionary<int, DispatchGroup>();
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
        }

        internal static void RegisterInstanceMethods(IntPtr luaState, int metatableIndex, Type type)
        {
            List<MethodInfo> methods = CollectMethods(type, isStatic: false);
            BindMethodGroups(luaState, metatableIndex, type, methods, isStatic: false);
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
                PushConstructorClosure(luaState, ctors[0]);
                LuaDll.lua_setfield(luaState, metatableIndex, "_ctor");
            }
            else
            {
                int[] ctorIds = new int[ctors.Length];
                for (int i = 0; i < ctors.Length; i++)
                {
                    ctorIds[i] = RegisterConstructor(ctors[i]);
                }

                PushDispatchClosure(luaState, new DispatchGroup
                {
                    IsStatic = true,
                    IsConstructor = true,
                    MethodName = "_ctor",
                    OwnerType = type,
                    ConstructorIds = ctorIds,
                });
                LuaDll.lua_setfield(luaState, metatableIndex, "_ctor");
            }

            RawGetField(luaState, metatableIndex, "_ctor");
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

            if (!method.IsSpecialName)
            {
                return true;
            }

            return false;
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
            int methodId = _nextMethodId++;
            if (isStatic)
            {
                StaticMethods[methodId] = method;
            }
            else
            {
                InstanceMethods[methodId] = method;
            }

            return methodId;
        }

        private static int RegisterConstructor(ConstructorInfo ctor)
        {
            int ctorId = _nextCtorId++;
            Constructors[ctorId] = ctor;
            return ctorId;
        }

        private static void PushMethodClosure(IntPtr luaState, MethodInfo method, bool isStatic)
        {
            int methodId = RegisterMethod(method, isStatic);
            LuaCSFunction cb = isStatic ? InvokeStaticMethod : InvokeInstanceMethod;
            CallbackRefs.Add(cb);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(cb);
            LuaDll.lua_pushinteger(luaState, methodId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        private static void PushConstructorClosure(IntPtr luaState, ConstructorInfo ctor)
        {
            int ctorId = RegisterConstructor(ctor);
            LuaCSFunction cb = InvokeConstructor;
            CallbackRefs.Add(cb);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(cb);
            LuaDll.lua_pushinteger(luaState, ctorId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        private static void PushDispatchClosure(IntPtr luaState, DispatchGroup group)
        {
            int dispatchId = _nextDispatchId++;
            DispatchGroups[dispatchId] = group;

            LuaCSFunction cb = group.IsConstructor
                ? DispatchConstructor
                : (group.IsStatic ? DispatchStaticMethod : DispatchInstanceMethod);
            CallbackRefs.Add(cb);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(cb);
            LuaDll.lua_pushinteger(luaState, dispatchId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeStaticMethod(IntPtr luaState)
        {
            int methodId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!StaticMethods.TryGetValue(methodId, out MethodInfo method))
            {
                return LuaDllExtension.error(luaState, $"novalua: static method id {methodId} not found");
            }

            return InvokeMethod(luaState, method, argStartIndex: 1, target: null);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeInstanceMethod(IntPtr luaState)
        {
            int methodId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!InstanceMethods.TryGetValue(methodId, out MethodInfo method))
            {
                return LuaDllExtension.error(luaState, $"novalua: instance method id {methodId} not found");
            }

            if (!TryGetUserDataTarget(luaState, 1, out object target))
            {
                return LuaDllExtension.error(luaState, $"novalua: invalid userdata for instance method {method.Name}");
            }

            return InvokeMethod(luaState, method, argStartIndex: 2, target);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DispatchStaticMethod(IntPtr luaState)
        {
            int dispatchId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!DispatchGroups.TryGetValue(dispatchId, out DispatchGroup group))
            {
                return LuaDllExtension.error(luaState, $"novalua: dispatch group {dispatchId} not found");
            }

            return DispatchMethods(luaState, group, argStartIndex: 1, target: null);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DispatchInstanceMethod(IntPtr luaState)
        {
            int dispatchId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!DispatchGroups.TryGetValue(dispatchId, out DispatchGroup group))
            {
                return LuaDllExtension.error(luaState, $"novalua: dispatch group {dispatchId} not found");
            }

            if (!TryGetUserDataTarget(luaState, 1, out object target))
            {
                return LuaDllExtension.error(luaState, $"novalua: invalid userdata for method {group.MethodName}");
            }

            return DispatchMethods(luaState, group, argStartIndex: 2, target);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int DispatchConstructor(IntPtr luaState)
        {
            int dispatchId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!DispatchGroups.TryGetValue(dispatchId, out DispatchGroup group) || !group.IsConstructor)
            {
                return LuaDllExtension.error(luaState, $"novalua: constructor dispatch group {dispatchId} not found");
            }

            int[] ctorIds = group.ConstructorIds;
            List<string> candidates = new List<string>(ctorIds.Length);
            for (int i = 0; i < ctorIds.Length; i++)
            {
                if (!Constructors.TryGetValue(ctorIds[i], out ConstructorInfo ctor))
                {
                    continue;
                }

                candidates.Add(GetMethodSignatureKey(ctor));
                if (TryBuildArguments(luaState, ctor, argStartIndex: 2, out object[] args))
                {
                    return InvokeConstructor(luaState, group.OwnerType, ctor, args);
                }
            }

            return LuaDllExtension.error(luaState, BuildNoOverloadMessage(group.OwnerType, "_ctor", luaState, argStartIndex: 2, candidates));
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int InvokeConstructor(IntPtr luaState)
        {
            int ctorId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!Constructors.TryGetValue(ctorId, out ConstructorInfo ctor))
            {
                return LuaDllExtension.error(luaState, $"novalua: constructor id {ctorId} not found");
            }

            if (!TryBuildArguments(luaState, ctor, argStartIndex: 2, out object[] args))
            {
                return LuaDllExtension.error(luaState,
                    $"novalua: constructor argument mismatch for {ctor.DeclaringType.FullName}{GetParameterSignature(ctor.GetParameters())}");
            }

            return InvokeConstructor(luaState, ctor.DeclaringType, ctor, args);
        }

        private static int DispatchMethods(IntPtr luaState, DispatchGroup group, int argStartIndex, object target)
        {
            int[] methodIds = group.MethodIds;
            List<string> candidates = new List<string>(methodIds.Length);
            for (int i = 0; i < methodIds.Length; i++)
            {
                MethodInfo method = group.IsStatic
                    ? StaticMethods[methodIds[i]]
                    : InstanceMethods[methodIds[i]];
                candidates.Add(GetMethodSignatureKey(method));
                if (TryBuildArguments(luaState, method, argStartIndex, out object[] args))
                {
                    return InvokePreparedMethod(luaState, method, target, args);
                }
            }

            return LuaDllExtension.error(luaState,
                BuildNoOverloadMessage(group.OwnerType, group.MethodName, luaState, argStartIndex, candidates));
        }

        private static int InvokeMethod(IntPtr luaState, MethodInfo method, int argStartIndex, object target)
        {
            if (!TryBuildArguments(luaState, method, argStartIndex, out object[] args))
            {
                return LuaDllExtension.error(luaState,
                    $"novalua: argument mismatch for {method.DeclaringType.FullName}.{method.Name}{GetParameterSignature(method.GetParameters())}");
            }

            return InvokePreparedMethod(luaState, method, target, args);
        }

        private static int InvokePreparedMethod(IntPtr luaState, MethodInfo method, object target, object[] args)
        {
            try
            {
                object ret = method.Invoke(target, args);
                return PushReturn(luaState, method.ReturnType, ret);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"novalua: method invoke error: {ex.InnerException?.Message ?? ex.Message}");
            }
        }

        private static int InvokeConstructor(IntPtr luaState, Type type, ConstructorInfo ctor, object[] args)
        {
            try
            {
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
                return LuaDllExtension.error(luaState, $"novalua: constructor invoke error: {ex.InnerException?.Message ?? ex.Message}");
            }
        }

        private static bool TryBuildArguments(IntPtr luaState, MethodBase method, int argStartIndex, out object[] args)
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
                    if (!TryReadArgument(luaState, argStartIndex + i, parameters[i].ParameterType, out object value))
                    {
                        return false;
                    }

                    args[i] = value;
                }

                Type elementType = parameters[paramsIndex].ParameterType.GetElementType();
                int varCount = luaArgCount - fixedCount;
                Array array = Array.CreateInstance(elementType, varCount);
                for (int i = 0; i < varCount; i++)
                {
                    if (!TryReadArgument(luaState, argStartIndex + fixedCount + i, elementType, out object value))
                    {
                        return false;
                    }

                    array.SetValue(value, i);
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
                    if (!TryReadArgument(luaState, argStartIndex + i, parameters[i].ParameterType, out object value))
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

        private static bool TryReadArgument(IntPtr luaState, int luaIndex, Type parameterType, out object value)
        {
            value = null;
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
                return !targetType.IsValueType || Nullable.GetUnderlyingType(parameterType) != null;
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
                || targetType == typeof(ushort) || targetType == typeof(sbyte))
            {
                return IsLuaInteger(luaState, luaIndex, targetType);
            }

            if (targetType == typeof(float) || targetType == typeof(double) || targetType == typeof(decimal))
            {
                return luaType == LuaDataType.Number;
            }

            if (targetType.IsClass || targetType.IsInterface)
            {
                return luaType == LuaDataType.UserData || luaType == LuaDataType.Nil;
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

            return $"no overload for {ownerType.FullName}.{methodName} matching ({string.Join(", ", argTags)}); candidates: {string.Join(", ", candidates)}";
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
            Type targetType = Nullable.GetUnderlyingType(type) ?? type;
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

            throw new NotSupportedException($"unsupported arg type {targetType.Name}");
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
    }
}
