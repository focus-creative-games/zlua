#pragma once

#include "NovaLuaCommon.h"
#include "vm/String.h"
#include "utils/StringUtils.h"

struct Il2CppString;

namespace novalua
{
    class Marshaling
    {
    public:
        static void LuaPCall(lua_State* L, int nargs, int nresults);

        static void PushCString(lua_State* L, Il2CppString* str);
        static Il2CppString* PopCString(lua_State* L, int idx);

        static void PushLightUserData(lua_State* L, void* ptr);
        static void* PopLightUserData(lua_State* L, int idx);

        static void PushUserData(lua_State* L, Il2CppObject* obj);
        static Il2CppObject* PopUserData(lua_State* L, int idx);
    };

    namespace detail
    {
        template<typename T>
        struct DefaultMarshaling;

        template<>
        struct DefaultMarshaling<void>
        {
            static void Push(lua_State*, void*) {}
            static void Pop(lua_State*, int) {}
        };

        template<>
        struct DefaultMarshaling<int32_t>
        {
            static void Push(lua_State* L, int32_t v) { lua_pushinteger(L, v); }
            static int32_t Pop(lua_State* L, int idx) { return (int32_t)lua_tointeger(L, idx); }
        };

        template<>
        struct DefaultMarshaling<int64_t>
        {
            static void Push(lua_State* L, int64_t v) { lua_pushinteger(L, v); }
            static int64_t Pop(lua_State* L, int idx) { return (int64_t)lua_tointeger(L, idx); }
        };

        template<>
        struct DefaultMarshaling<bool>
        {
            static void Push(lua_State* L, bool v) { lua_pushboolean(L, v ? 1 : 0); }
            static bool Pop(lua_State* L, int idx) { return lua_toboolean(L, idx) != 0; }
        };

        template<>
        struct DefaultMarshaling<float>
        {
            static void Push(lua_State* L, float v) { lua_pushnumber(L, v); }
            static float Pop(lua_State* L, int idx) { return (float)lua_tonumber(L, idx); }
        };

        template<>
        struct DefaultMarshaling<double>
        {
            static void Push(lua_State* L, double v) { lua_pushnumber(L, v); }
            static double Pop(lua_State* L, int idx) { return lua_tonumber(L, idx); }
        };

        template<>
        struct DefaultMarshaling<Il2CppString*>
        {
            static void Push(lua_State* L, Il2CppString* v);
            static Il2CppString* Pop(lua_State* L, int idx);
        };

        template<typename T>
        IL2CPP_FORCE_INLINE void PushDefault(lua_State* L, T value)
        {
            DefaultMarshaling<T>::Push(L, value);
        }

        template<typename T>
        IL2CPP_FORCE_INLINE T PopDefault(lua_State* L, int idx)
        {
            return DefaultMarshaling<T>::Pop(L, idx);
        }
    } // namespace detail
} // namespace novalua
