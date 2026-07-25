#pragma once

#include "../ZLuaCommon.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"

#include "vm/Object.h"

namespace zlua
{

class InstanceTarget
{
  public:
    static UserDataKind GetUserDataKind(lua_State* L, int index);
    static UserDataInfo GetUserDataInfo(lua_State* L, int index);

    static inline void* ResolveStaticMethodTarget(lua_State* L, int index)
    {
        return nullptr;
    }

    static inline void* ResolveByValMethodTarget(lua_State* L, int index)
    {
        ByValUserDataHeader* header = StructMarshal::GetByValHeaderUnchecked(L, index);
        IL2CPP_ASSERT(header != nullptr);
        return header->Payload();
    }

    static inline void* ResolveByObjValueTypeMethodTarget(lua_State* L, int index)
    {
        Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, index);
        IL2CPP_ASSERT(obj != nullptr);
        return reinterpret_cast<uint8_t*>(obj) + sizeof(Il2CppObject);
    }

    static inline Il2CppObject* ResolveBoxedByValMethodTarget(lua_State* L, int index)
    {
        ByValUserDataHeader* header = StructMarshal::GetByValHeaderUnchecked(L, index);
        IL2CPP_ASSERT(header != nullptr);
        return il2cpp::vm::Object::Box(header->klass, header->Payload());
    }

    static inline Il2CppObject* ResolveBoxedByObjValueTypeMethodTarget(lua_State* L, int index)
    {
        Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, index);
        IL2CPP_ASSERT(obj != nullptr);
        return obj;
    }

    static inline void* ResolveByObjReferenceMethodTarget(lua_State* L, int index)
    {
        Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, index);
        IL2CPP_ASSERT(obj != nullptr);
        return obj;
    }

    static FnResolveMethodThis GetResolveMethodThisFunc(Il2CppClass* klass, bool isStatic, bool isByVal);

    static inline void* ResolveByValFieldAddress(lua_State* L, int index, int32_t payloadRelativeOffset)
    {
        ByValUserDataHeader* header = static_cast<ByValUserDataHeader*>(lua_touserdata(L, index));
        IL2CPP_ASSERT(header != nullptr);
        return header->Payload() + payloadRelativeOffset;
    }

    static inline void* ResolveByObjFieldAddress(lua_State* L, int index, int32_t payloadRelativeOffset)
    {
        Il2CppObject* obj = ObjectMarshal::PopByObjThis(L, index);
        IL2CPP_ASSERT(obj != nullptr);
        return reinterpret_cast<uint8_t*>(obj) + payloadRelativeOffset;
    }
};
} // namespace zlua
