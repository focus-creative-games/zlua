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
    Il2CppClass* klass = obj->klass;
    // if (klass == il2cpp_defaults.string_class)
    // {
    //     StringMarshal::Push(L, (Il2CppString*)obj);
    //     return;
    // }
    int metatableRefIndex = obj->klass == meta->typeKlass ? MarshalMeta::EnsureByObjMetatableRef(L, meta) : LUA_NOREF;
    // if (MetadataUtil::IsDelegateClass(klass))
    // {
    //     DelegateMarshal::Push(L, (Il2CppDelegate*)obj, metatableRefIndex);
    //     return;
    // }

    ObjectRegistry::Push(L, obj, metatableRefIndex);
}

void ObjectMarshal::Push(lua_State* L, Il2CppObject* obj)
{
    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }
    Il2CppClass* klass = obj->klass;
    if (klass == il2cpp_defaults.string_class)
    {
        StringMarshal::Push(L, (Il2CppString*)obj);
        return;
    }
    int metatableRefIndex = LUA_NOREF;
    // if (MetadataUtil::IsDelegateClass(klass))
    // {
    //     DelegateMarshal::Push(L, (Il2CppDelegate*)obj, metatableRefIndex);
    //     return;
    // }

    ObjectRegistry::Push(L, obj, metatableRefIndex);
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
        if (klass != il2cpp_defaults.object_class)
        {
            LuaException::ThrowFormat("zlua argument mismatch: boolean value can only be assigned to object type");
        }
        Il2CppClass* booleanClass = il2cpp_defaults.boolean_class;
        bool value = lua_toboolean(L, objIndex);
        return il2cpp::vm::Object::Box(booleanClass, &value);
    }
    case LUA_TNUMBER:
    {
        if (klass != il2cpp_defaults.object_class)
        {
            LuaException::ThrowFormat("zlua argument mismatch: number value can only be assigned to object type");
        }
        if (lua_isinteger(L, objIndex))
        {
            int64_t value = (int64_t)lua_tointeger(L, objIndex);
            Il2CppClass* numberClass = il2cpp_defaults.int32_class;
            return il2cpp::vm::Object::Box(numberClass, &value);
        }
        else
        {
            double doubleValue = lua_tonumber(L, objIndex);
            Il2CppClass* numberClass = il2cpp_defaults.double_class;
            return il2cpp::vm::Object::Box(numberClass, &doubleValue);
        }
    }
    case LUA_TSTRING:
    {
        if (klass != il2cpp_defaults.object_class && klass != il2cpp_defaults.string_class)
        {
            LuaException::ThrowFormat("zlua argument mismatch: string value can only be assigned to object or string type");
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

Il2CppObject* ObjectMarshal::PopNotDelegate(lua_State* L, int objIndex, Il2CppClass* klass)
{
    return Pop(L, objIndex, klass);
}
} // namespace zlua