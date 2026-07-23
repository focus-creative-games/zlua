using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using ZLua.Utils;

namespace ZLua.Marshaling
{
    /// <summary>
    /// (obj, viewType) weak cache + slot strong refs. Mirrors Il2Cpp <c>ObjectRegistry</c>.
    /// </summary>
    internal static class ObjectRegistry
    {
        private const uint InvalidSlotIndex = uint.MaxValue;

        private static int s_objectCacheRef = LuaConsts.LuaNoRef;
        private static readonly Dictionary<ObjectViewKey, int> s_objectViewRefs = new Dictionary<ObjectViewKey, int>();
        private static readonly ObjectSlotRegistry s_objectSlots = new ObjectSlotRegistry();
        private static readonly LuaCSFunction s_onRelease = OnReleaseObjectUserData;
        private static bool s_releasePinned;

        private readonly struct ObjectViewKey : IEquatable<ObjectViewKey>
        {
            public readonly object Obj;
            public readonly Type ViewType;

            public ObjectViewKey(object obj, Type viewType)
            {
                Obj = obj;
                ViewType = viewType;
            }

            public bool Equals(ObjectViewKey other) =>
                ReferenceEquals(Obj, other.Obj) && ViewType == other.ViewType;

            public override bool Equals(object obj) => obj is ObjectViewKey other && Equals(other);

            public override int GetHashCode()
            {
                unchecked
                {
                    int h = RuntimeHelpersHash(Obj);
                    h = (h * 397) ^ (ViewType != null ? ViewType.GetHashCode() : 0);
                    return h;
                }
            }

            private static int RuntimeHelpersHash(object obj) =>
                System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(obj);
        }

        private sealed class ObjectSlotRegistry
        {
            private const int InitialCapacity = 1024;
            private ObjectSlot[] _slots = Array.Empty<ObjectSlot>();
            private int _nextSlotIndex;
            private readonly Stack<uint> _freeSlots = new Stack<uint>();

            public uint Register(object obj, Type viewType)
            {
                uint slotIndex = AllocateSlot();
                _slots[slotIndex] = new ObjectSlot { Obj = obj, ViewType = viewType };
                return slotIndex;
            }

            public void Unregister(uint slotIndex)
            {
                if (slotIndex >= (uint)_nextSlotIndex)
                {
                    return;
                }

                _slots[slotIndex] = default;
                _freeSlots.Push(slotIndex);
            }

            public ObjectSlot Get(uint slotIndex)
            {
                if (slotIndex >= (uint)_nextSlotIndex)
                {
                    return default;
                }

                return _slots[slotIndex];
            }

            private uint AllocateSlot()
            {
                if (_freeSlots.Count > 0)
                {
                    return _freeSlots.Pop();
                }

                if (_nextSlotIndex >= _slots.Length)
                {
                    EnsureCapacity(_nextSlotIndex + 1);
                }

                return (uint)_nextSlotIndex++;
            }

            private void EnsureCapacity(int minCapacity)
            {
                if (minCapacity <= _slots.Length)
                {
                    return;
                }

                int newCapacity = _slots.Length == 0 ? InitialCapacity : _slots.Length;
                while (newCapacity < minCapacity)
                {
                    newCapacity *= 2;
                }

                Array.Resize(ref _slots, newCapacity);
            }
        }

        private struct ObjectSlot
        {
            public object Obj;
            public Type ViewType;
        }

        internal static void Initialize(IntPtr L)
        {
            if (s_objectCacheRef != LuaConsts.LuaNoRef)
            {
                return;
            }

            if (!s_releasePinned)
            {
                // Keep delegate alive for native Lua __gc.
                GCHandle.Alloc(s_onRelease);
                s_releasePinned = true;
            }

            LuaDll.lua_createtable(L, 0, 0);
            LuaDll.lua_createtable(L, 0, 1);
            LuaDll.lua_pushstring(L, LuaConsts.WeakModeValue);
            LuaDll.lua_setfield(L, -2, LuaConsts.MetaMode);
            LuaDll.lua_setmetatable(L, -2);
            s_objectCacheRef = LuaDll.luaL_ref(L, LuaConsts.LuaRegistryIndex);
        }

        internal static void Shutdown(IntPtr L)
        {
            s_objectViewRefs.Clear();
            if (s_objectCacheRef != LuaConsts.LuaNoRef)
            {
                LuaDll.luaL_unref(L, LuaConsts.LuaRegistryIndex, s_objectCacheRef);
                s_objectCacheRef = LuaConsts.LuaNoRef;
            }
        }

        internal static void Push(IntPtr L, object obj, Type viewType, int metatableRefIndex)
        {
            if (obj == null)
            {
                LuaDll.lua_pushnil(L);
                return;
            }

            if (viewType == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: viewType is null");
            }

            if (TryPushCachedObject(L, obj, viewType))
            {
                return;
            }

            unsafe
            {
                IntPtr udPtr = LuaDll.lua_newuserdatauv(L, (UIntPtr)sizeof(ZLuaObjectUserData), 0);
                ZLuaObjectUserData* ud = (ZLuaObjectUserData*)udPtr;
                ud->Header.Kind = UserDataKind.ByObj;
                ud->SlotIndex = s_objectSlots.Register(obj, viewType);
            }

            AttachObjectMetatable(L, viewType, metatableRefIndex);
            AddToObjectCache(L, obj, viewType, -1);
        }

        internal static object Pop(IntPtr L, int idx)
        {
            if (LuaDll.lua_isnil(L, idx))
            {
                return null;
            }

            unsafe
            {
                ZLuaObjectUserData* ud = (ZLuaObjectUserData*)LuaDll.lua_touserdata(L, idx);
                if (ud == null || ud->Header.Kind != UserDataKind.ByObj)
                {
                    LuaCallbackBoundary.Throw("zlua argument mismatch: expected by-obj userdata");
                }

                return s_objectSlots.Get(ud->SlotIndex).Obj;
            }
        }

        internal static object PopThis(IntPtr L, int idx)
        {
            unsafe
            {
                ZLuaObjectUserData* ud = (ZLuaObjectUserData*)LuaDll.lua_touserdata(L, idx);
                return s_objectSlots.Get(ud->SlotIndex).Obj;
            }
        }

        internal static Type GetViewType(IntPtr L, int idx)
        {
            unsafe
            {
                ZLuaObjectUserData* ud = (ZLuaObjectUserData*)LuaDll.lua_touserdata(L, idx);
                if (ud == null || ud->Header.Kind != UserDataKind.ByObj)
                {
                    return null;
                }

                return s_objectSlots.Get(ud->SlotIndex).ViewType;
            }
        }

        [MonoLuaCallback(typeof(LuaCSFunction))]
        private static int OnReleaseObjectUserData(IntPtr L)
        {
            unsafe
            {
                ZLuaObjectUserData* ud = (ZLuaObjectUserData*)LuaDll.lua_touserdata(L, 1);
                if (ud == null || ud->SlotIndex == InvalidSlotIndex)
                {
                    return 0;
                }

                ObjectSlot slot = s_objectSlots.Get(ud->SlotIndex);
                s_objectSlots.Unregister(ud->SlotIndex);
                ud->SlotIndex = InvalidSlotIndex;
                RemoveFromObjectCache(L, slot.Obj, slot.ViewType);
            }

            return 0;
        }

        internal static IntPtr GetOnReleaseFunctionPointer()
        {
            return global::System.Runtime.InteropServices.Marshal.GetFunctionPointerForDelegate(s_onRelease);
        }

        private static void AttachObjectMetatable(IntPtr L, Type viewType, int metatableRefIndex)
        {
            if (metatableRefIndex != LuaConsts.LuaNoRef)
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, metatableRefIndex);
                LuaDll.lua_setmetatable(L, -2);
                return;
            }

            Action<IntPtr, Type> push = MetatableHooks.PushByObjMetatable;
            if (push == null)
            {
                LuaCallbackBoundary.Throw("zlua internal error: ByObj metatable hook not registered");
            }

            push(L, viewType);
            LuaDll.lua_setmetatable(L, -2);
        }

        private static bool TryPushCachedObject(IntPtr L, object obj, Type viewType)
        {
            ObjectViewKey key = new ObjectViewKey(obj, viewType);
            if (!s_objectViewRefs.TryGetValue(key, out int cacheKey))
            {
                return false;
            }

            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, s_objectCacheRef);
            LuaDll.lua_rawgeti(L, -1, cacheKey);
            LuaDll.lua_remove(L, -2);

            if (!LuaDll.lua_isuserdata(L, -1))
            {
                LuaDll.lua_pop(L, 1);
                InvalidateCacheEntry(L, key, cacheKey);
                return false;
            }

            unsafe
            {
                ZLuaObjectUserData* ud = (ZLuaObjectUserData*)LuaDll.lua_touserdata(L, -1);
                ObjectSlot slot = ud != null ? s_objectSlots.Get(ud->SlotIndex) : default;
                if (ud == null
                    || !ReferenceEquals(slot.Obj, obj)
                    || slot.ViewType != viewType
                    || ud->SlotIndex == InvalidSlotIndex)
                {
                    LuaDll.lua_pop(L, 1);
                    InvalidateCacheEntry(L, key, cacheKey);
                    return false;
                }
            }

            return true;
        }

        private static void AddToObjectCache(IntPtr L, object obj, Type viewType, int userdataIndex)
        {
            int absUserdataIndex = LuaDll.lua_absindex(L, userdataIndex);
            ObjectViewKey key = new ObjectViewKey(obj, viewType);

            if (s_objectViewRefs.TryGetValue(key, out int existing))
            {
                LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, s_objectCacheRef);
                LuaDll.luaL_unref(L, -1, existing);
                LuaDll.lua_pop(L, 1);
                s_objectViewRefs.Remove(key);
            }

            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, s_objectCacheRef);
            LuaDll.lua_pushvalue(L, absUserdataIndex);
            int cacheKey = LuaDll.luaL_ref(L, -2);
            LuaDll.lua_pop(L, 1);
            s_objectViewRefs[key] = cacheKey;
        }

        private static void RemoveFromObjectCache(IntPtr L, object obj, Type viewType)
        {
            if (obj == null || viewType == null || s_objectCacheRef == LuaConsts.LuaNoRef)
            {
                return;
            }

            ObjectViewKey key = new ObjectViewKey(obj, viewType);
            if (!s_objectViewRefs.TryGetValue(key, out int cacheKey))
            {
                return;
            }

            s_objectViewRefs.Remove(key);
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, s_objectCacheRef);
            LuaDll.luaL_unref(L, -1, cacheKey);
            LuaDll.lua_pop(L, 1);
        }

        private static void InvalidateCacheEntry(IntPtr L, ObjectViewKey key, int cacheKey)
        {
            s_objectViewRefs.Remove(key);
            LuaDll.lua_rawgeti(L, LuaConsts.LuaRegistryIndex, s_objectCacheRef);
            LuaDll.luaL_unref(L, -1, cacheKey);
            LuaDll.lua_pop(L, 1);
        }
    }
}
