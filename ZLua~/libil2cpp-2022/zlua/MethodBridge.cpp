#include "MethodBridge.h"

#include "Marshaling.h"
#include "ObjectRegistry.h"

namespace zlua
{
    int MethodBridge::InvokeStaticIntIntInt(lua_State* L, const MethodInfo* method, int argStart)
    {
        const int32_t a = (int32_t)lua_tointeger(L, argStart);
        const int32_t b = (int32_t)lua_tointeger(L, argStart + 1);
        typedef int32_t (*Fn)(int32_t, int32_t);
        Fn fn = (Fn)method->methodPointer;
        const int32_t result = fn(a, b);
        lua_pushinteger(L, result);
        return 1;
    }

    int MethodBridge::InvokeStaticInt(lua_State* L, const MethodInfo* method)
    {
        typedef int32_t (*Fn)();
        Fn fn = (Fn)method->methodPointer;
        const int32_t result = fn();
        lua_pushinteger(L, result);
        return 1;
    }

    int MethodBridge::InvokeStaticVoidInt(lua_State* L, const MethodInfo* method, int argStart)
    {
        const int32_t value = (int32_t)lua_tointeger(L, argStart);
        typedef void (*Fn)(int32_t);
        Fn fn = (Fn)method->methodPointer;
        fn(value);
        return 0;
    }

    int MethodBridge::InvokeInstanceInt(lua_State* L, const MethodInfo* method, Il2CppObject* instance, int argStart)
    {
        (void)argStart;
        typedef int32_t (*Fn)(Il2CppObject*);
        Fn fn = (Fn)method->methodPointer;
        const int32_t result = fn(instance);
        lua_pushinteger(L, result);
        return 1;
    }

    int MethodBridge::InvokeInstanceVoidInt(lua_State* L, const MethodInfo* method, Il2CppObject* instance, int argStart)
    {
        const int32_t value = (int32_t)lua_tointeger(L, argStart);
        typedef void (*Fn)(Il2CppObject*, int32_t);
        Fn fn = (Fn)method->methodPointer;
        fn(instance, value);
        return 0;
    }

    int MethodBridge::InvokeConstructorNoArgs(lua_State* L, const MethodInfo* ctor, Il2CppClass* klass)
    {
        (void)ctor;
        Il2CppObject* instance = il2cpp::vm::Object::New(klass);
        if (instance == nullptr)
            return luaL_error(L, "zlua: failed to create instance");

        ObjectRegistry::PushObject(L, instance);
        return 1;
    }
}
