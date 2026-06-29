using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    /// <summary>
    /// (argc, per-slot Lua type) key for overload fast dispatch (P1 · 4.6).
    /// </summary>
    internal static class OverloadTypeMask
    {
        internal readonly struct FastDispatchEntry
        {
            internal readonly MethodBase Method;
            internal readonly LuaCSFunction Invoker;

            internal FastDispatchEntry(MethodBase method, LuaCSFunction invoker)
            {
                Method = method;
                Invoker = invoker;
            }
        }

        internal const int MaxTrackedArgs = 8;
        private const int SlotShift = 32;
        private const int SlotBits = 4;

        internal static ulong FromStack(IntPtr luaState, int argStartIndex)
        {
            int top = LuaDll.lua_gettop(luaState);
            int argc = top >= argStartIndex ? top - argStartIndex + 1 : 0;
            return PackFromStack(argc, luaState, argStartIndex);
        }

        internal static ulong PackFromStack(int argc, IntPtr luaState, int argStartIndex)
        {
            ulong key = (ulong)(uint)argc;
            int count = Math.Min(argc, MaxTrackedArgs);
            for (int i = 0; i < count; i++)
            {
                key |= (ulong)(uint)LuaDll.lua_type(luaState, argStartIndex + i) << (SlotShift + i * SlotBits);
            }

            return key;
        }

        internal static bool TryBuildFastMap(
            IReadOnlyList<MethodInfo> methods,
            IReadOnlyList<LuaCSFunction> invokers,
            out Dictionary<ulong, FastDispatchEntry> map)
        {
            map = new Dictionary<ulong, FastDispatchEntry>();
            if (methods == null || invokers == null || methods.Count != invokers.Count || methods.Count == 0)
            {
                return false;
            }

            var collisions = new HashSet<ulong>();
            for (int i = 0; i < methods.Count; i++)
            {
                MethodInfo method = methods[i];
                LuaCSFunction invoker = invokers[i];
                if (method == null || invoker == null)
                {
                    return false;
                }

                if (!TryEnumerateExpectedKeys(method, out List<ulong> keys))
                {
                    return false;
                }

                var entry = new FastDispatchEntry(method, invoker);
                for (int k = 0; k < keys.Count; k++)
                {
                    ulong key = keys[k];
                    if (map.TryGetValue(key, out FastDispatchEntry existing))
                    {
                        if (!ReferenceEquals(existing.Invoker, invoker))
                        {
                            collisions.Add(key);
                        }
                    }
                    else
                    {
                        map[key] = entry;
                    }
                }
            }

            foreach (ulong collision in collisions)
            {
                map.Remove(collision);
            }

            return map.Count > 0;
        }

        internal static bool TryBuildConstructorFastMap(
            IReadOnlyList<ConstructorInfo> constructors,
            IReadOnlyList<LuaCSFunction> invokers,
            out Dictionary<ulong, FastDispatchEntry> map)
        {
            map = new Dictionary<ulong, FastDispatchEntry>();
            if (constructors == null || invokers == null || constructors.Count != invokers.Count || constructors.Count == 0)
            {
                return false;
            }

            var collisions = new HashSet<ulong>();
            for (int i = 0; i < constructors.Count; i++)
            {
                ConstructorInfo ctor = constructors[i];
                LuaCSFunction invoker = invokers[i];
                if (ctor == null || invoker == null)
                {
                    return false;
                }

                if (!TryEnumerateExpectedKeys(ctor, out List<ulong> keys))
                {
                    return false;
                }

                var entry = new FastDispatchEntry(ctor, invoker);
                for (int k = 0; k < keys.Count; k++)
                {
                    ulong key = keys[k];
                    if (map.TryGetValue(key, out FastDispatchEntry existing))
                    {
                        if (!ReferenceEquals(existing.Invoker, invoker))
                        {
                            collisions.Add(key);
                        }
                    }
                    else
                    {
                        map[key] = entry;
                    }
                }
            }

            foreach (ulong collision in collisions)
            {
                map.Remove(collision);
            }

            return map.Count > 0;
        }

        internal static int TryInvokeFast(
            IntPtr luaState,
            Dictionary<ulong, FastDispatchEntry> map,
            int argStartIndex)
        {
            if (map == null || map.Count == 0)
            {
                return -1;
            }

            ulong key = FromStack(luaState, argStartIndex);
            if (!map.TryGetValue(key, out FastDispatchEntry entry))
            {
                return -1;
            }

            if (entry.Method is MethodInfo methodInfo)
            {
                bool expectStatic = argStartIndex <= 1;
                if (methodInfo.IsStatic != expectStatic)
                {
                    return -1;
                }
            }

            if (!TypeMethodRegistration.TryCanConvertArguments(luaState, entry.Method, argStartIndex))
            {
                return -1;
            }

            return entry.Invoker(luaState);
        }

        private static bool TryEnumerateExpectedKeys(MethodBase method, out List<ulong> keys)
        {
            keys = new List<ulong>();
            ParameterInfo[] parameters = method.GetParameters();
            if (FindParamsIndex(parameters) >= 0)
            {
                return false;
            }

            for (int i = 0; i < parameters.Length; i++)
            {
                if (HasNonDefaultMarshal(parameters[i], method))
                {
                    return false;
                }
            }

            int minArgCount = 0;
            for (int i = 0; i < parameters.Length; i++)
            {
                if (!parameters[i].HasDefaultValue)
                {
                    minArgCount = i + 1;
                }
            }

            for (int argc = minArgCount; argc <= parameters.Length; argc++)
            {
                if (!TryPackExpectedKey(method, parameters, argc, out ulong key))
                {
                    return false;
                }

                keys.Add(key);
            }

            return keys.Count > 0;
        }

        private static bool TryPackExpectedKey(
            MethodBase method,
            ParameterInfo[] parameters,
            int argc,
            out ulong key)
        {
            key = (ulong)(uint)argc;
            for (int i = 0; i < argc; i++)
            {
                if (!TryGetExpectedSlotType(parameters[i], method, out LuaDataType slotType))
                {
                    return false;
                }

                key |= (ulong)(uint)slotType << (SlotShift + i * SlotBits);
            }

            return true;
        }

        private static bool TryGetExpectedSlotType(ParameterInfo parameter, MethodBase method, out LuaDataType slotType)
        {
            slotType = LuaDataType.Nil;
            Type parameterType = Nullable.GetUnderlyingType(parameter.ParameterType) ?? parameter.ParameterType;
            if (parameterType.IsByRef)
            {
                parameterType = parameterType.GetElementType();
            }

            if (parameterType == typeof(bool))
            {
                slotType = LuaDataType.Boolean;
                return true;
            }

            if (parameterType == typeof(string))
            {
                slotType = LuaDataType.String;
                return true;
            }

            if (parameterType == typeof(int) || parameterType == typeof(long)
                || parameterType == typeof(short) || parameterType == typeof(byte)
                || parameterType == typeof(uint) || parameterType == typeof(ulong)
                || parameterType == typeof(ushort) || parameterType == typeof(sbyte)
                || parameterType == typeof(char) || parameterType == typeof(float)
                || parameterType == typeof(double) || parameterType.IsEnum)
            {
                slotType = LuaDataType.Number;
                return true;
            }

            if (parameterType == typeof(IntPtr) || parameterType == typeof(UIntPtr))
            {
                slotType = LuaDataType.Number;
                return true;
            }

            if (parameterType == typeof(object))
            {
                return false;
            }

            if (typeof(Delegate).IsAssignableFrom(parameterType))
            {
                slotType = LuaDataType.Function;
                return true;
            }

            if (parameterType.IsArray)
            {
                slotType = LuaDataType.UserData;
                return true;
            }

            if (ValueTypeMarshaling.IsStructType(parameterType))
            {
                return false;
            }

            if (parameterType.IsClass || parameterType.IsInterface)
            {
                slotType = LuaDataType.UserData;
                return true;
            }

            return false;
        }

        private static bool HasNonDefaultMarshal(ParameterInfo parameter, MethodBase method)
        {
            LuaMarshalType marshalType = LuaMarshalAsResolver.ResolveParameter(
                parameter,
                method,
                LuaMarshalDirection.LuaToCSharp,
                parameter.ParameterType);
            return marshalType != LuaMarshalType.Default;
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
    }
}
