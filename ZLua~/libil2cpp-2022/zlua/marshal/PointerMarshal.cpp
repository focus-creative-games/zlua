#include "PointerMarshal.h"

#include "../MetadataUtil.h"
#include "../LuaUtil.h"
#include "Marshaling.h"
#include "PrimitiveMarshal.h"

#include "il2cpp-class-internals.h"
#include "il2cpp-tabledefs.h"
#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/Type.h"

#include <cstring>

namespace zlua
{
    bool PointerMarshal::IsIntPtrClass(Il2CppClass* klass)
    {
        return klass == il2cpp_defaults.int_class;
    }

    bool PointerMarshal::IsUIntPtrClass(Il2CppClass* klass)
    {
        return klass == il2cpp_defaults.uint_class;
    }

    bool PointerMarshal::IsPointerType(const Il2CppType* type)
    {
        return type != nullptr && type->type == IL2CPP_TYPE_PTR;
    }

    static bool IsByRefLikeClass(Il2CppClass* klass)
    {
        if (klass == nullptr || !klass->byval_arg.valuetype)
            return false;

        const char* fullName = MetadataUtil::GetTypeFullName(klass);
        if (fullName == nullptr)
            return false;

        return strncmp(fullName, "System.Span`", 12) == 0
            || strncmp(fullName, "System.ReadOnlySpan`", 20) == 0;
    }

    static bool IsSystemDecimal(Il2CppClass* klass)
    {
        return (klass->image == il2cpp_defaults.corlib && strcmp(klass->namespaze, "System") == 0 && strcmp(klass->name, "Decimal") == 0);
    }

    bool PointerMarshal::IsUnsupportedMarshalType(Il2CppClass* klass)
    {
        if (klass == nullptr)
            return false;

        if (klass == il2cpp_defaults.typed_reference_class
            || IsSystemDecimal(klass))
            return true;

        return IsByRefLikeClass(klass);
    }

    bool PointerMarshal::CanConvert(lua_State* L, int index, const Il2CppType* type)
    {
        if (type == nullptr)
            return false;

        if (IsPointerType(type))
        {
            const int luaType = lua_type(L, index);
            return luaType == LUA_TNIL || luaType == LUA_TLIGHTUSERDATA;
        }

        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type, false);
        if (klass == nullptr)
            return false;

        if (IsIntPtrClass(klass) || IsUIntPtrClass(klass))
            return LuaUtil::IsStrictLuaInteger(L, index) || lua_isnumber(L, index);

        return false;
    }

    bool PointerMarshal::TryPop(lua_State* L, int index, const Il2CppType* type, void* dest, size_t destSize)
    {
        if (dest == nullptr || type == nullptr)
            return false;

        if (IsPointerType(type))
        {
            if (destSize < sizeof(void*))
                return false;

            if (lua_type(L, index) == LUA_TNIL)
            {
                *reinterpret_cast<void**>(dest) = nullptr;
                return true;
            }

            if (lua_type(L, index) != LUA_TLIGHTUSERDATA)
                return false;

            *reinterpret_cast<void**>(dest) = lua_touserdata(L, index);
            return true;
        }

        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(type, false);
        if (klass == nullptr)
            return false;

        il2cpp::vm::Class::Init(klass);
        const size_t sz = il2cpp::vm::Class::GetValueSize(klass, nullptr);
        if (destSize < sz)
            return false;

        if (IsIntPtrClass(klass))
        {
            const intptr_t value = (intptr_t)PrimitiveMarshal::PopInt64(L, index);
            std::memcpy(dest, &value, sz);
            return true;
        }

        if (IsUIntPtrClass(klass))
        {
            const uintptr_t value = (uintptr_t)PrimitiveMarshal::PopUInt64(L, index);
            std::memcpy(dest, &value, sz);
            return true;
        }

        return false;
    }

    static void* ReadNativePointerFromBoxed(Il2CppObject* retObj, Il2CppClass* klass)
    {
        if (retObj == nullptr || klass == nullptr)
            return nullptr;

        if (PointerMarshal::IsIntPtrClass(klass) || PointerMarshal::IsUIntPtrClass(klass))
            return *reinterpret_cast<void**>(il2cpp::vm::Object::Unbox(retObj));

        return nullptr;
    }

    int PointerMarshal::PushReturn(lua_State* L, const Il2CppType* returnType, Il2CppObject* retObj)
    {
        if (returnType == nullptr)
            return 0;

        if (IsPointerType(returnType))
        {
            void* ptr = retObj != nullptr ? reinterpret_cast<void*>(retObj) : nullptr;
            Marshaling::PushLightUserData(L, ptr);
            return 1;
        }

        Il2CppClass* klass = il2cpp::vm::Class::FromIl2CppType(returnType, false);
        if (klass == nullptr)
            return 0;

        il2cpp::vm::Class::Init(klass);
        if (IsIntPtrClass(klass) || IsUIntPtrClass(klass))
        {
            if (retObj == nullptr)
            {
                lua_pushinteger(L, 0);
                return 1;
            }

            void* ptr = ReadNativePointerFromBoxed(retObj, klass);
            lua_pushinteger(L, (lua_Integer)(intptr_t)ptr);
            return 1;
        }

        return 0;
    }
}
