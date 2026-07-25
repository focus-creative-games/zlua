#pragma once

#include "../ZLuaCommon.h"
#include "../marshal/MarshalDefs.h"

namespace zlua
{
struct TypeBinding;

class TypeRegistryCommon
{
  public:
    static Il2CppObject* GetByObjThis(lua_State* L, int index);
    static void RegisterNativeInstanceMethod(lua_State* L, TypeBinding* binding, const char* name, lua_CFunction fn);
    static int TypeTableToString(lua_State* L);
    static int ObjectInstanceToString(lua_State* L);
    static void WriteCommonTypeFields(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding);
    static void RegisterStaticLiteralFields(lua_State* L, Il2CppClass* klass, int typeTableIndex);
    static void AttachStaticTypeMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction callFn, lua_CFunction extraFn,
                                          const char* extraFieldName, TypeBinding* binding);
    static void AttachReferenceInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, TypeBinding* binding);
    static void AttachByValInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding);
    static void AttachByObjInstanceMetatable(lua_State* L, Il2CppClass* klass, int typeTableIndex, lua_CFunction tostring, TypeBinding* binding);
};
} // namespace zlua
