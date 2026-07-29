#include "ObjectMarshal.h"

#include "ObjectRegistry.h"
#include "DelegateMarshal.h"
#include "MarshalMeta.h"

#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaUtil.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/ArrayMarshal.h"

#include "vm/Class.h"
#include "vm/Object.h"

namespace zlua
{

void ObjectMarshal::Push(lua_State* L, Il2CppObject* obj, const MarshalMetaInfo* meta)
{
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    IL2CPP_ASSERT(meta != nullptr);
    Il2CppClass* viewKlass = meta->typeKlass;
    IL2CPP_ASSERT(viewKlass != nullptr);
    /* Declared-type facade only; never switch to runtime klass or special marshal here. */
    int metatableRefIndex = MarshalMeta::EnsureByObjMetatableRef(L, meta);
    ObjectRegistry::Push(L, obj, viewKlass, metatableRefIndex);
}

void ObjectMarshal::Push(lua_State* L, Il2CppObject* obj, Il2CppClass* viewKlass)
{
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    IL2CPP_ASSERT(viewKlass != nullptr);
    ObjectRegistry::Push(L, obj, viewKlass, LUA_NOREF);
}

void ObjectMarshal::Push(lua_State* L, Il2CppObject* obj)
{
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    /* No MarshalMeta: facade is the instance runtime class (ctors / zlua helpers). */
    ObjectRegistry::Push(L, obj, obj->klass, LUA_NOREF);
}

Il2CppObject* ObjectMarshal::Pop(lua_State* L, int objIndex, Il2CppClass* klass)
{
    int type = lua_type(L, objIndex);
    switch (type)
    {
    case LUA_TNIL:
    {
        return nullptr;
    }
    case LUA_TUSERDATA:
    {
        Il2CppObject* obj = ObjectRegistry::Pop(L, objIndex);
        if (obj != nullptr && !il2cpp::vm::Class::IsAssignableFrom(klass, obj->klass))
        {
            LuaException::ThrowFormat("zlua argument mismatch: object is not of type: %s.%s", klass->namespaze, klass->name);
        }
        return obj;
    }
    case LUA_TFUNCTION:
    {
        if (MetadataUtil::IsDelegateClass(klass))
        {
            return (Il2CppObject*)DelegateMarshal::Pop(L, objIndex, klass);
        }
        break;
    }
    case LUA_TBOOLEAN:
    {
        Il2CppClass* booleanClass = il2cpp_defaults.boolean_class;
        if (!il2cpp::vm::Class::IsAssignableFrom(klass, booleanClass))
        {
            LuaException::ThrowFormat("zlua argument mismatch: boolean value can only be assigned to boolean type");
        }
        bool value = lua_toboolean(L, objIndex);
        return il2cpp::vm::Object::Box(booleanClass, &value);
    }
    case LUA_TNUMBER:
    {
        if (lua_isinteger(L, objIndex))
        {
            int64_t wide = (int64_t)lua_tointeger(L, objIndex);
            if (wide >= (int64_t)INT32_MIN && wide <= (int64_t)INT32_MAX)
            {
                Il2CppClass* numberClass = il2cpp_defaults.int32_class;
                if (!il2cpp::vm::Class::IsAssignableFrom(klass, numberClass))
                {
                    LuaException::ThrowFormat("zlua argument mismatch: number value can only be assigned to int32 type");
                }
                int32_t value = (int32_t)wide;
                return il2cpp::vm::Object::Box(numberClass, &value);
            }

            Il2CppClass* numberClass = il2cpp_defaults.int64_class;
            if (!il2cpp::vm::Class::IsAssignableFrom(klass, numberClass))
            {
                LuaException::ThrowFormat("zlua argument mismatch: integer out of int32 range requires int64-compatible target");
            }
            return il2cpp::vm::Object::Box(numberClass, &wide);
        }
        else
        {
            double doubleValue = lua_tonumber(L, objIndex);
            Il2CppClass* numberClass = il2cpp_defaults.double_class;
            if (!il2cpp::vm::Class::IsAssignableFrom(klass, numberClass))
            {
                LuaException::ThrowFormat("zlua argument mismatch: number value can only be assigned to double type");
            }
            return il2cpp::vm::Object::Box(numberClass, &doubleValue);
        }
    }
    case LUA_TSTRING:
    {
        Il2CppClass* stringClass = il2cpp_defaults.string_class;
        if (!il2cpp::vm::Class::IsAssignableFrom(klass, stringClass))
        {
            LuaException::ThrowFormat("zlua argument mismatch: string value can only be assigned to string type");
        }
        return (Il2CppObject*)StringMarshal::Pop(L, objIndex);
    }
    case LUA_TTABLE:
    {
        if (klass->rank == 0)
        {
            LuaException::ThrowFormat("zlua argument mismatch: table value can only be assigned to array type");
        }
        return (Il2CppObject*)ArrayMarshal::PopFromTable(L, objIndex, klass);
    }
    default:
        break;
    }
    LuaException::ThrowFormat("zlua argument mismatch: unsupported object type: %s", lua_typename(L, type));
}
} // namespace zlua
