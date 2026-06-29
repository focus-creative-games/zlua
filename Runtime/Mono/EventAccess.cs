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
        internal sealed class EventBinding
        {
            internal EventInfo Event;
            internal bool IsStatic;
            internal MethodInfo AddMethod;
            internal MethodInfo RemoveMethod;
            internal MethodInfo RaiseMethod;
            internal LuaCSFunction FastAdd;
            internal LuaCSFunction FastRemove;
            internal LuaCSFunction FastFire;
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

        internal static int RegisterEventBinding(EventInfo eventInfo, bool isStatic)
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

        internal static void PushEventTable(IntPtr luaState, int eventId)
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
                int eventId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Events.TryGetValue(eventId, out EventBinding binding))
                {
                    LuaCallbackBoundary.Throw("zlua: invalid event fire invocation");
                }

                if (binding.FastFire != null)
                {
                    return binding.FastFire(luaState);
                }

                if (!TryResolveBinding(luaState, binding, out object target, out int argStartIndex, requireHandler: false))
                {
                    LuaCallbackBoundary.Throw("zlua: invalid event fire invocation");
                }

                MethodInfo raiseMethod = binding.RaiseMethod;
                if (raiseMethod == null)
                {
                    LuaCallbackBoundary.Throw($"zlua: event {binding.Event.Name} has no raise method");
                }

                if (!TypeMethodRegistration.TryBuildArguments(luaState, raiseMethod, argStartIndex, out object[] args))
                {
                    LuaCallbackBoundary.Throw(BuildEventArgumentError(binding.Event, raiseMethod));
                }

                object result = raiseMethod.Invoke(target, args);
                return TypeMethodRegistration.PushReturnValue(luaState, raiseMethod.ReturnType, result);
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        private static int MutateSubscription(IntPtr luaState, bool isAdd)
        {
            try
            {
                int eventId = (int)LuaDll.lua_tointeger(luaState, LuaConsts.LuaRegistryIndex - 1);
                if (!Events.TryGetValue(eventId, out EventBinding binding))
                {
                    LuaCallbackBoundary.Throw("zlua: invalid event subscription");
                }

                if (!TryResolveBinding(luaState, binding, out object target, out int handlerIndex, requireHandler: true))
                {
                    LuaCallbackBoundary.Throw("zlua: invalid event subscription");
                }

                Type handlerType = binding.Event.EventHandlerType;
                if (LuaDll.lua_type(luaState, handlerIndex) == LuaDataType.Function)
                {
                    IntPtr functionPointer = LuaDll.lua_topointer(luaState, handlerIndex);
                    if (functionPointer == IntPtr.Zero)
                    {
                        LuaCallbackBoundary.Throw($"zlua: event {binding.Event.Name} handler is not a valid Lua function");
                    }

                    EventSubscriptionKey key = new EventSubscriptionKey(target, eventId, functionPointer);
                    if (isAdd)
                    {
                        if (!EventHandlerCache.TryGetValue(key, out Delegate handler))
                        {
                            handler = ReadEventHandler(luaState, handlerIndex, handlerType, binding.Event.Name);
                            if (handler == null)
                            {
                                LuaCallbackBoundary.Throw($"zlua: event {binding.Event.Name} handler cannot be nil");
                            }

                            EventHandlerCache[key] = handler;
                        }

                        InvokeAddHandler(binding, target, handler);
                    }
                    else
                    {
                        if (!EventHandlerCache.TryGetValue(key, out Delegate handler))
                        {
                            LuaCallbackBoundary.Throw(
                                $"zlua: event {binding.Event.Name} handler was not registered through get");
                        }

                        InvokeRemoveHandler(binding, target, handler);
                        EventHandlerCache.Remove(key);
                    }
                }
                else
                {
                    if (isAdd && binding.FastAdd != null)
                    {
                        return binding.FastAdd(luaState);
                    }

                    if (!isAdd && binding.FastRemove != null)
                    {
                        return binding.FastRemove(luaState);
                    }

                    Delegate handler = ReadEventHandler(luaState, handlerIndex, handlerType, binding.Event.Name);
                    if (handler == null)
                    {
                        LuaCallbackBoundary.Throw($"zlua: event {binding.Event.Name} handler cannot be nil");
                    }

                    if (isAdd)
                    {
                        InvokeAddHandler(binding, target, handler);
                    }
                    else
                    {
                        InvokeRemoveHandler(binding, target, handler);
                    }
                }

                return 0;
            }
            catch (Exception ex)
            {
                return LuaCallbackBoundary.ToLuaError(luaState, ex);
            }
        }

        private static void InvokeAddHandler(EventBinding binding, object target, Delegate handler)
        {
            if (binding.AddMethod != null)
            {
                binding.AddMethod.Invoke(target, new object[] { handler });
                return;
            }

            binding.Event.AddEventHandler(target, handler);
        }

        private static void InvokeRemoveHandler(EventBinding binding, object target, Delegate handler)
        {
            if (binding.RemoveMethod != null)
            {
                binding.RemoveMethod.Invoke(target, new object[] { handler });
                return;
            }

            binding.Event.RemoveEventHandler(target, handler);
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
            EventBinding binding,
            out object target,
            out int handlerOrArgStartIndex,
            bool requireHandler)
        {
            target = null;
            handlerOrArgStartIndex = 0;

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

        private static int RegisterEvent(EventInfo eventInfo, bool isStatic)
        {
            int eventId = _nextEventId++;
            MethodInfo raiseMethod = ResolveRaiseMethod(eventInfo, isStatic);
            MethodInfo addMethod = eventInfo.GetAddMethod(nonPublic: false);
            MethodInfo removeMethod = eventInfo.GetRemoveMethod(nonPublic: false);

            EventBinding binding = new EventBinding
            {
                Event = eventInfo,
                IsStatic = isStatic,
                AddMethod = addMethod,
                RemoveMethod = removeMethod,
                RaiseMethod = raiseMethod,
            };

            if (LuaToCSharpEventBridgeFactory.CanFastEvent(eventInfo, isStatic))
            {
                if (LuaToCSharpEventBridgeFactory.TryCreateAdd(eventInfo, isStatic, out LuaCSFunction fastAdd))
                {
                    binding.FastAdd = fastAdd;
                    CallbackRefs.Add(fastAdd);
                }

                if (LuaToCSharpEventBridgeFactory.TryCreateRemove(eventInfo, isStatic, out LuaCSFunction fastRemove))
                {
                    binding.FastRemove = fastRemove;
                    CallbackRefs.Add(fastRemove);
                }

                if (raiseMethod != null
                    && LuaToCSharpEventBridgeFactory.TryCreateFire(eventInfo, raiseMethod, isStatic, out LuaCSFunction fastFire))
                {
                    binding.FastFire = fastFire;
                    CallbackRefs.Add(fastFire);
                }
            }

            Events[eventId] = binding;
            return eventId;
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
