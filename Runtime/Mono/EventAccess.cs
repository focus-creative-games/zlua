using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace ZLua
{
    /// <summary>
    /// C# event 在 Lua 侧暴露为 { get = add, set = remove, fire? = raise } 表（TYPE_SYSTEM_SPEC §4.5）。
    /// </summary>
    internal static class EventAccess
    {
        private sealed class EventBinding
        {
            internal EventInfo Event;
            internal bool IsStatic;
            internal MethodInfo RaiseMethod;
        }

        private readonly struct EventSubscriptionKey : IEquatable<EventSubscriptionKey>
        {
            internal readonly object Target;
            internal readonly int EventId;
            internal readonly IntPtr LuaFunctionPointer;

            internal EventSubscriptionKey(object target, int eventId, IntPtr luaFunctionPointer)
            {
                Target = target;
                EventId = eventId;
                LuaFunctionPointer = luaFunctionPointer;
            }

            public bool Equals(EventSubscriptionKey other)
            {
                return EventId == other.EventId
                    && LuaFunctionPointer == other.LuaFunctionPointer
                    && ReferenceEquals(Target, other.Target);
            }

            public override bool Equals(object obj)
            {
                return obj is EventSubscriptionKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                return HashCode.Combine(RuntimeHelpers.GetHashCode(Target), EventId, LuaFunctionPointer.ToInt64());
            }
        }

        private static readonly Dictionary<int, EventBinding> Events = new Dictionary<int, EventBinding>();
        private static readonly Dictionary<(int MetadataToken, bool IsStatic), int> EventIdCache =
            new Dictionary<(int, bool), int>();
        private static readonly Dictionary<EventSubscriptionKey, Delegate> EventHandlerCache =
            new Dictionary<EventSubscriptionKey, Delegate>();
        private static readonly List<LuaCSFunction> CallbackRefs = new List<LuaCSFunction>();
        private static int _nextEventId = 1;

        internal static void RegisterStaticEvents(IntPtr luaState, int metatableIndex, Type type)
        {
            HashSet<string> registered = new HashSet<string>(StringComparer.Ordinal);
            for (Type current = type; current != null; current = current.BaseType)
            {
                EventInfo[] events = current.GetEvents(BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly);
                Array.Sort(events, (a, b) => string.CompareOrdinal(a.Name, b.Name));
                for (int i = 0; i < events.Length; i++)
                {
                    EventInfo eventInfo = events[i];
                    if (!registered.Add(eventInfo.Name))
                    {
                        continue;
                    }

                    PushEventTable(luaState, GetOrRegisterEventId(eventInfo, isStatic: true));
                    LuaDll.lua_setfield(luaState, metatableIndex, eventInfo.Name);
                }
            }
        }

        internal static bool TryPushInstanceEvent(IntPtr luaState, Type type, string name, object target)
        {
            for (Type current = type; current != null; current = current.BaseType)
            {
                EventInfo eventInfo = current.GetEvent(
                    name,
                    BindingFlags.Public | BindingFlags.Instance | BindingFlags.DeclaredOnly);
                if (eventInfo == null)
                {
                    continue;
                }

                PushEventTable(luaState, GetOrRegisterEventId(eventInfo, isStatic: false));
                return true;
            }

            return false;
        }

        private static int GetOrRegisterEventId(EventInfo eventInfo, bool isStatic)
        {
            (int, bool) key = (eventInfo.MetadataToken, isStatic);
            if (EventIdCache.TryGetValue(key, out int eventId))
            {
                return eventId;
            }

            eventId = RegisterEvent(eventInfo, isStatic);
            EventIdCache[key] = eventId;
            return eventId;
        }

        private static int RegisterEvent(EventInfo eventInfo, bool isStatic)
        {
            int eventId = _nextEventId++;
            Events[eventId] = new EventBinding
            {
                Event = eventInfo,
                IsStatic = isStatic,
                RaiseMethod = ResolveRaiseMethod(eventInfo, isStatic),
            };
            return eventId;
        }

        private static void PushEventTable(IntPtr luaState, int eventId)
        {
            LuaDll.lua_createtable(luaState, 0, 3);

            PushEventCallback(luaState, eventId, EventAdd);
            LuaDll.lua_setfield(luaState, -2, "get");

            PushEventCallback(luaState, eventId, EventRemove);
            LuaDll.lua_setfield(luaState, -2, "set");

            if (Events[eventId].RaiseMethod != null)
            {
                PushEventCallback(luaState, eventId, EventFire);
                LuaDll.lua_setfield(luaState, -2, "fire");
            }
        }

        private static MethodInfo ResolveRaiseMethod(EventInfo eventInfo, bool isStatic)
        {
            MethodInfo raiseMethod = eventInfo.GetRaiseMethod(nonPublic: false);
            if (raiseMethod != null)
            {
                return raiseMethod;
            }

            Type declaringType = eventInfo.DeclaringType;
            if (declaringType == null)
            {
                return null;
            }

            BindingFlags flags = BindingFlags.Public | BindingFlags.DeclaredOnly
                | (isStatic ? BindingFlags.Static : BindingFlags.Instance);
            return declaringType.GetMethod("Raise" + eventInfo.Name, flags);
        }

        private static void PushEventCallback(IntPtr luaState, int eventId, LuaCSFunction callback)
        {
            CallbackRefs.Add(callback);
            IntPtr fn = Marshal.GetFunctionPointerForDelegate(callback);
            LuaDll.lua_pushinteger(luaState, eventId);
            LuaDll.lua_pushcclosure(luaState, fn, 1);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int EventAdd(IntPtr luaState)
        {
            return MutateSubscription(luaState, isAdd: true);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int EventRemove(IntPtr luaState)
        {
            return MutateSubscription(luaState, isAdd: false);
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int EventFire(IntPtr luaState)
        {
            try
            {
                if (!TryResolveBinding(luaState, out EventBinding binding, out object target, out int argStartIndex, requireHandler: false))
                {
                    return LuaDllExtension.error(luaState, "zlua: invalid event fire invocation");
                }

                MethodInfo raiseMethod = binding.RaiseMethod;
                if (raiseMethod == null)
                {
                    return LuaDllExtension.error(luaState, $"zlua: event {binding.Event.Name} has no raise method");
                }

                if (!TypeMethodRegistration.TryBuildArguments(luaState, raiseMethod, argStartIndex, out object[] args))
                {
                    return LuaDllExtension.error(luaState, BuildEventArgumentError(binding.Event, raiseMethod));
                }

                object result = raiseMethod.Invoke(target, args);
                return TypeMethodRegistration.PushReturnValue(luaState, raiseMethod.ReturnType, result);
            }
            catch (Exception ex)
            {
                return LuaDllExtension.error(luaState, $"zlua event fire error: {ex.InnerException?.Message ?? ex.Message}");
            }
        }

        private static int MutateSubscription(IntPtr luaState, bool isAdd)
        {
            try
            {
                int eventId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!TryResolveBinding(luaState, out EventBinding binding, out object target, out int handlerIndex, requireHandler: true))
                {
                    return LuaDllExtension.error(luaState, "zlua: invalid event subscription");
                }

                Type handlerType = binding.Event.EventHandlerType;
                Delegate handler;
                if (LuaDll.lua_type(luaState, handlerIndex) == LuaDataType.Function)
                {
                    IntPtr functionPointer = LuaDll.lua_topointer(luaState, handlerIndex);
                    if (functionPointer == IntPtr.Zero)
                    {
                        return LuaDllExtension.error(luaState, $"zlua: event {binding.Event.Name} handler is not a valid Lua function");
                    }

                    EventSubscriptionKey key = new EventSubscriptionKey(target, eventId, functionPointer);
                    if (isAdd)
                    {
                        if (!EventHandlerCache.TryGetValue(key, out handler))
                        {
                            handler = ReadEventHandler(luaState, handlerIndex, handlerType, binding.Event.Name);
                            if (handler == null)
                            {
                                return LuaDllExtension.error(luaState, $"zlua: event {binding.Event.Name} handler cannot be nil");
                            }

                            EventHandlerCache[key] = handler;
                        }

                        binding.Event.AddEventHandler(target, handler);
                    }
                    else
                    {
                        if (!EventHandlerCache.TryGetValue(key, out handler))
                        {
                            return LuaDllExtension.error(
                                luaState,
                                $"zlua: event {binding.Event.Name} handler was not registered through get");
                        }

                        binding.Event.RemoveEventHandler(target, handler);
                        EventHandlerCache.Remove(key);
                    }
                }
                else
                {
                    handler = ReadEventHandler(luaState, handlerIndex, handlerType, binding.Event.Name);
                    if (handler == null)
                    {
                        return LuaDllExtension.error(luaState, $"zlua: event {binding.Event.Name} handler cannot be nil");
                    }

                    if (isAdd)
                    {
                        binding.Event.AddEventHandler(target, handler);
                    }
                    else
                    {
                        binding.Event.RemoveEventHandler(target, handler);
                    }
                }

                return 0;
            }
            catch (Exception ex)
            {
                string verb = isAdd ? "add" : "remove";
                return LuaDllExtension.error(luaState, $"zlua event {verb} error: {ex.InnerException?.Message ?? ex.Message}");
            }
        }

        private static Delegate ReadEventHandler(IntPtr luaState, int handlerIndex, Type handlerType, string eventName)
        {
            object handlerValue = TypeMethodRegistration.ReadArgumentValue(luaState, handlerIndex, handlerType);
            if (handlerValue == null)
            {
                return null;
            }

            if (handlerValue is Delegate handler)
            {
                return handler;
            }

            throw new NotSupportedException($"zlua: event {eventName} expects delegate {handlerType.Name}");
        }

        private static bool TryResolveBinding(
            IntPtr luaState,
            out EventBinding binding,
            out object target,
            out int handlerOrArgStartIndex,
            bool requireHandler)
        {
            binding = null;
            target = null;
            handlerOrArgStartIndex = 0;

            int eventId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
            if (!Events.TryGetValue(eventId, out binding))
            {
                return false;
            }

            if (binding.IsStatic)
            {
                target = null;
                handlerOrArgStartIndex = 1;
                if (requireHandler && LuaDll.lua_gettop(luaState) < handlerOrArgStartIndex)
                {
                    return false;
                }

                return !requireHandler || LuaDll.lua_type(luaState, handlerOrArgStartIndex) != LuaDataType.Nil;
            }

            if (LuaDll.lua_gettop(luaState) < 1)
            {
                return false;
            }

            if (!TryGetUserDataTarget(luaState, 1, out target))
            {
                return false;
            }

            if (requireHandler)
            {
                if (LuaDll.lua_gettop(luaState) < 2)
                {
                    return false;
                }

                handlerOrArgStartIndex = 2;
                return LuaDll.lua_type(luaState, handlerOrArgStartIndex) != LuaDataType.Nil;
            }

            handlerOrArgStartIndex = 2;
            return true;
        }

        private static bool TryGetUserDataTarget(IntPtr luaState, int index, out object target)
        {
            return ValueTypeMarshaling.TryGetBoxedTarget(luaState, index, out target);
        }

        private static string BuildEventArgumentError(EventInfo eventInfo, MethodBase method)
        {
            return $"zlua: invalid arguments for event {eventInfo.DeclaringType?.FullName}.{eventInfo.Name} fire ({GetParameterSignature(method.GetParameters())})";
        }

        private static string GetParameterSignature(ParameterInfo[] parameters)
        {
            if (parameters == null || parameters.Length == 0)
            {
                return "()";
            }

            var sb = new StringBuilder(parameters.Length * 16);
            sb.Append('(');
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
    }
}