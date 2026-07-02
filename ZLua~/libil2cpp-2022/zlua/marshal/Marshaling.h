#pragma once

#include <cstdint>

#include "../ZLuaCommon.h"

#include "PrimitiveMarshal.h"
#include "StringMarshal.h"
#include "ObjectMarshal.h"

struct Il2CppString;

namespace zlua
{
    class Marshaling
    {
    public:
        static void LuaPCall(lua_State* L, int nargs, int nresults, int errfunc = 0);

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
            static void Push(lua_State* L, int32_t v) { PrimitiveMarshal::PushInt32(L, v); }
            static int32_t Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopInt32(L, idx); }
        };

        template<>
        struct DefaultMarshaling<int64_t>
        {
            static void Push(lua_State* L, int64_t v) { PrimitiveMarshal::PushInt64(L, v); }
            static int64_t Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopInt64(L, idx); }
        };

        template<>
        struct DefaultMarshaling<uint32_t>
        {
            static void Push(lua_State* L, uint32_t v) { PrimitiveMarshal::PushUInt32(L, v); }
            static uint32_t Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopUInt32(L, idx); }
        };

        template<>
        struct DefaultMarshaling<uint64_t>
        {
            static void Push(lua_State* L, uint64_t v) { PrimitiveMarshal::PushUInt64(L, v); }
            static uint64_t Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopUInt64(L, idx); }
        };

        template<>
        struct DefaultMarshaling<bool>
        {
            static void Push(lua_State* L, bool v) { PrimitiveMarshal::PushBool(L, v); }
            static bool Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopBool(L, idx); }
        };

        template<>
        struct DefaultMarshaling<float>
        {
            static void Push(lua_State* L, float v) { PrimitiveMarshal::PushFloat(L, v); }
            static float Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopFloat(L, idx); }
        };

        template<>
        struct DefaultMarshaling<double>
        {
            static void Push(lua_State* L, double v) { PrimitiveMarshal::PushDouble(L, v); }
            static double Pop(lua_State* L, int idx) { return PrimitiveMarshal::PopDouble(L, idx); }
        };

        template<>
        struct DefaultMarshaling<Il2CppString*>
        {
            static void Push(lua_State* L, Il2CppString* v) { StringMarshal::Push(L, v); }
            static Il2CppString* Pop(lua_State* L, int idx) { return StringMarshal::Pop(L, idx); }
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
} // namespace zlua
