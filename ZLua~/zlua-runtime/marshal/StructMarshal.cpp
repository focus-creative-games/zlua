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

#include "StructMarshal.h"
#include "StructRegistry.h"
#include "TypedMarshal.h"

#include "../LuaConsts.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaStackGuard.h"
#include "../utils/LuaException.h"
#include "../mt/MetaTableCache.h"
#include "../bridge/FieldBridge.h"

#include "gc/GarbageCollector.h"
#include "vm/Class.h"
#include "vm/Type.h"
#include "vm/Object.h"

namespace zlua
{

ByValUserDataHeader* StructMarshal::GetByValHeader(lua_State* L, int index)
{
    UserDataHeader* header = (UserDataHeader*)lua_touserdata(L, index);
    if (header == nullptr || header->kind != UserDataKind::ByVal)
        return nullptr;
    return (ByValUserDataHeader*)header;
}

static ByValUserDataHeader* CreateByValUserDataHeader(lua_State* L, Il2CppClass* klass, size_t payloadSize, int metatableRefIndex)
{
    const size_t totalSize = sizeof(ByValUserDataHeader) + payloadSize;
    ByValUserDataHeader* header = static_cast<ByValUserDataHeader*>(LuaNewUserData(L, totalSize));
    header->header.kind = UserDataKind::ByVal;
    header->klass = klass;
    if (metatableRefIndex != LUA_NOREF)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, metatableRefIndex);
        lua_setmetatable(L, -2);
    }
    else
    {
        MetaTableCache::PushByValMetatable(L, klass);
        lua_setmetatable(L, -2);
    }
    return header;
}

void StructMarshal::PushValue(lua_State* L, void* dataAddr, Il2CppClass* klass, int metatableRefIndex)
{
    const size_t payloadSize = (size_t)MetadataUtil::GetInstanceSizeWithoutHeader(klass);
    ByValUserDataHeader* header = CreateByValUserDataHeader(L, klass, payloadSize, metatableRefIndex);
    std::memcpy(header->Payload(), dataAddr, payloadSize);
    if (!klass->is_blittable)
        StructRegistry::Register(header);
}

void* StructMarshal::PushZeroedValue(lua_State* L, Il2CppClass* klass)
{
    IL2CPP_ASSERT(klass != nullptr);
    const size_t payloadSize = (size_t)MetadataUtil::GetInstanceSizeWithoutHeader(klass);
    ByValUserDataHeader* header = CreateByValUserDataHeader(L, klass, payloadSize, LUA_NOREF);
    void* payload = header->Payload();
    std::memset(payload, 0, payloadSize);
    if (!klass->is_blittable)
        StructRegistry::Register(header);
    return payload;
}

void StructMarshal::PopValue(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr)
{
    IL2CPP_ASSERT(!klass->enumtype && !klass->nullabletype);
    ByValUserDataHeader* header = GetByValHeader(L, index);
    if (header == nullptr)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert non-by-val userdata to struct: %s.%s", klass->namespaze, klass->name);

    if (klass != header->klass)
        LuaException::ThrowFormat("zlua argument mismatch: cannot convert userdata %s.%s to struct: %s.%s", header->klass->namespaze, header->klass->name,
                                  klass->namespaze, klass->name);

    const size_t payloadSize = MetadataUtil::GetInstanceSizeWithoutHeader(klass);
    std::memcpy(outDataAddr, header->Payload(), payloadSize);
    if (!klass->is_blittable)
    {
        il2cpp::gc::GarbageCollector::SetWriteBarrier((void**)outDataAddr, payloadSize);
    }
}

void StructMarshal::PushNullableValue(lua_State* L, void* dataAddr, Il2CppClass* klass, int metatableRefIndex)
{
    if (!il2cpp::vm::Object::NullableHasValue(klass, dataAddr))
    {
        lua_pushnil(L);
        return;
    }
    IL2CPP_ASSERT(metatableRefIndex != LUA_NOREF);

    void* ptr = MetadataUtil::GetNullableValue(dataAddr, klass);
    Il2CppClass* elementClass = klass->element_class;
    if (elementClass->enumtype)
    {
        TypedMarshal::PushByType(L, ptr, il2cpp::vm::Class::GetEnumBaseType(elementClass));
        return;
    }
    Il2CppTypeEnum typeEnum = elementClass->byval_arg.type;
    if (typeEnum == IL2CPP_TYPE_VALUETYPE || typeEnum == IL2CPP_TYPE_GENERICINST)
    {
        PushValue(L, ptr, elementClass, metatableRefIndex);
        return;
    }

    TypedMarshal::PushByType(L, ptr, &elementClass->byval_arg);
}

void StructMarshal::PopNullableValue(lua_State* L, int index, Il2CppClass* klass, void* outDataAddr)
{
    int type = lua_type(L, index);
    if (type == LUA_TNIL)
    {
        MetadataUtil::InitNullableValue(outDataAddr, klass);
        return;
    }
    Il2CppClass* elementClass = klass->element_class;
    void* valueAddr = MetadataUtil::GetNullableValue(outDataAddr, klass);
    TypedMarshal::PopByType(L, index, valueAddr, elementClass->enumtype ? il2cpp::vm::Class::GetEnumBaseType(elementClass) : &elementClass->byval_arg);
    MetadataUtil::NullableSetHasValue(outDataAddr, klass);
}

} // namespace zlua
