// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
