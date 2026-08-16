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

#include "TypeRegistry.h"

#include "TypeRegistryArray.h"
#include "MetaBinding.h"
#include "TypeRegistryReference.h"
#include "TypeRegistryValueType.h"

#include "../LuaConsts.h"
#include "../utils/MetadataUtil.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"

#include "vm/Class.h"
#include "vm/Runtime.h"
#include "metadata/GenericMetadata.h"

#include <unordered_map>

namespace zlua
{
static std::unordered_map<Il2CppClass*, int> s_internedTypeTableRefs;

static void PushTypeTable(lua_State* L, Il2CppClass* klass)
{
    il2cpp::vm::Class::Init(klass);
    
    if (!klass->is_generic && !il2cpp::metadata::GenericMetadata::ContainsGenericParameters(klass))
    {
        il2cpp::vm::Runtime::ClassInit(klass);
    }
    const Il2CppType* type = &klass->byval_arg;
    switch (type->type)
    {
    case IL2CPP_TYPE_BOOLEAN:
    case IL2CPP_TYPE_CHAR:
    case IL2CPP_TYPE_I1:
    case IL2CPP_TYPE_U1:
    case IL2CPP_TYPE_I2:
    case IL2CPP_TYPE_U2:
    case IL2CPP_TYPE_I4:
    case IL2CPP_TYPE_U4:
    case IL2CPP_TYPE_I8:
    case IL2CPP_TYPE_U8:
    case IL2CPP_TYPE_R4:
    case IL2CPP_TYPE_R8:
    case IL2CPP_TYPE_I:
    case IL2CPP_TYPE_U:
    case IL2CPP_TYPE_TYPEDBYREF:
    {
        TypeRegistryValueType::CreateTypeTable(L, klass);
        break;
    }
    case IL2CPP_TYPE_STRING:
    case IL2CPP_TYPE_OBJECT:
    case IL2CPP_TYPE_CLASS:
    {
        TypeRegistryReference::CreateTypeTable(L, klass);
        break;
    }
    case IL2CPP_TYPE_ARRAY:
    case IL2CPP_TYPE_SZARRAY:
    {
        TypeRegistryArray::CreateTypeTable(L, klass);
        break;
    }
    case IL2CPP_TYPE_VALUETYPE:
    case IL2CPP_TYPE_GENERICINST:
    {
        if (MetadataUtil::IsValueTypeClass(klass))
        {
            if (klass->enumtype)
            {
                TypeRegistryValueType::CreateEnumTypeTable(L, klass);
            }
            else if (klass->nullabletype)
            {
                TypeRegistryValueType::CreateNullableTypeTable(L, klass);
            }
            else
            {
                TypeRegistryValueType::CreateTypeTable(L, klass);
            }
        }
        else
        {
            TypeRegistryReference::CreateTypeTable(L, klass);
        }
        break;
    }
    case IL2CPP_TYPE_PTR:
    case IL2CPP_TYPE_FNPTR:
    {
        LuaException::Throw("zlua: unsupported type: PTR or FNPTR");
        break;
    }
    default:
    {
        LuaException::Throw("zlua: unsupported type");
        break;
    }
    }
}

void TypeRegistry::PushInternedTypeTable(lua_State* L, Il2CppClass* klass)
{
    auto it = s_internedTypeTableRefs.find(klass);
    if (it != s_internedTypeTableRefs.end())
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
        return;
    }

    PushTypeTable(L, klass);
    lua_pushvalue(L, -1);
    const int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    s_internedTypeTableRefs[klass] = ref;
}

Il2CppClass* TypeRegistry::GetClassFromTypeTable(lua_State* L, int index)
{
    if (!lua_istable(L, index))
        LuaException::Throw("zlua: type table expected");
    lua_getfield(L, index, LuaConsts::Klass);
    Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return klass;
}
} // namespace zlua
