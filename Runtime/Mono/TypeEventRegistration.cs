using System;
using System.Collections.Generic;
using System.Reflection;

namespace ZLua
{
    internal static class TypeEventRegistration
    {
        private readonly struct EventKey : IEquatable<EventKey>
        {
            internal readonly int TypeId;
            internal readonly string Name;
            internal readonly bool IsStatic;

            internal EventKey(int typeId, string name, bool isStatic)
            {
                TypeId = typeId;
                Name = name;
                IsStatic = isStatic;
            }

            public bool Equals(EventKey other)
            {
                return TypeId == other.TypeId
                    && IsStatic == other.IsStatic
                    && string.Equals(Name, other.Name, StringComparison.Ordinal);
            }

            public override bool Equals(object obj)
            {
                return obj is EventKey other && Equals(other);
            }

            public override int GetHashCode()
            {
                unchecked
                {
                    int hash = TypeId;
                    hash = (hash * 397) ^ (IsStatic ? 1 : 0);
                    hash = (hash * 397) ^ (Name?.GetHashCode() ?? 0);
                    return hash;
                }
            }
        }

        private static readonly Dictionary<EventKey, int> StaticEventIdsByKey = new Dictionary<EventKey, int>();
        private static readonly Dictionary<EventKey, int> InstanceEventIdsByKey = new Dictionary<EventKey, int>();

        internal static void RegisterEvents(IntPtr luaState, int staticMethodTableIndex, Type type, int typeId)
        {
            if (type == null || type.IsEnum)
            {
                return;
            }

            List<EventInfo> staticEvents = CollectEvents(type, isStatic: true);
            for (int i = 0; i < staticEvents.Count; i++)
            {
                RegisterStaticEvent(luaState, staticMethodTableIndex, staticEvents[i], typeId);
            }

            List<EventInfo> instanceEvents = CollectEvents(type, isStatic: false);
            for (int i = 0; i < instanceEvents.Count; i++)
            {
                RegisterInstanceEvent(instanceEvents[i], typeId);
            }
        }

        internal static void BindInstanceEventsToMethodTable(IntPtr luaState, int methodTableIndex, int typeId)
        {
            int absTableIndex = LuaDll.lua_absindex(luaState, methodTableIndex);
            foreach (KeyValuePair<EventKey, int> entry in InstanceEventIdsByKey)
            {
                EventKey key = entry.Key;
                if (key.TypeId != typeId || key.IsStatic)
                {
                    continue;
                }

                EventAccess.PushEventTable(luaState, entry.Value);
                LuaDll.lua_setfield(luaState, absTableIndex, key.Name);
            }
        }

        internal static int TryPushStaticEvent(IntPtr luaState, int typeId, string eventName)
        {
            if (string.IsNullOrEmpty(eventName))
            {
                return -1;
            }

            EventKey key = new EventKey(typeId, eventName, isStatic: true);
            if (!StaticEventIdsByKey.TryGetValue(key, out int eventId))
            {
                return -1;
            }

            EventAccess.PushEventTable(luaState, eventId);
            return 1;
        }

        internal static int TryPushInstanceEvent(IntPtr luaState, int typeId, string eventName)
        {
            if (string.IsNullOrEmpty(eventName))
            {
                return -1;
            }

            EventKey key = new EventKey(typeId, eventName, isStatic: false);
            if (!InstanceEventIdsByKey.TryGetValue(key, out int eventId))
            {
                return -1;
            }

            EventAccess.PushEventTable(luaState, eventId);
            return 1;
        }

        private static void RegisterStaticEvent(IntPtr luaState, int staticMethodTableIndex, EventInfo eventInfo, int typeId)
        {
            EventKey key = new EventKey(typeId, eventInfo.Name, isStatic: true);
            if (StaticEventIdsByKey.ContainsKey(key))
            {
                return;
            }

            int eventId = EventAccess.RegisterEventBinding(eventInfo, isStatic: true);
            StaticEventIdsByKey[key] = eventId;
            EventAccess.PushEventTable(luaState, eventId);
            LuaDll.lua_setfield(luaState, staticMethodTableIndex, eventInfo.Name);
        }

        private static void RegisterInstanceEvent(EventInfo eventInfo, int typeId)
        {
            EventKey key = new EventKey(typeId, eventInfo.Name, isStatic: false);
            if (InstanceEventIdsByKey.ContainsKey(key))
            {
                return;
            }

            int eventId = EventAccess.RegisterEventBinding(eventInfo, isStatic: false);
            InstanceEventIdsByKey[key] = eventId;
        }

        private static List<EventInfo> CollectEvents(Type type, bool isStatic)
        {
            BindingFlags flags = (isStatic ? BindingFlags.Static : BindingFlags.Instance)
                | BindingFlags.Public
                | BindingFlags.DeclaredOnly;

            Dictionary<string, EventInfo> byName = new Dictionary<string, EventInfo>(StringComparer.Ordinal);
            for (Type current = type; current != null; current = current.BaseType)
            {
                EventInfo[] events = current.GetEvents(flags);
                Array.Sort(events, (a, b) => string.CompareOrdinal(a.Name, b.Name));
                for (int i = 0; i < events.Length; i++)
                {
                    EventInfo eventInfo = events[i];
                    if (!byName.ContainsKey(eventInfo.Name))
                    {
                        byName[eventInfo.Name] = eventInfo;
                    }
                }
            }

            List<EventInfo> result = new List<EventInfo>(byName.Count);
            foreach (KeyValuePair<string, EventInfo> kv in byName)
            {
                result.Add(kv.Value);
            }

            return result;
        }
    }
}
