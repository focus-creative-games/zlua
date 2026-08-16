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

#include "InstanceTarget.h"

#include "../utils/LuaException.h"
#include "../utils/MetadataUtil.h"

namespace zlua
{
UserDataKind InstanceTarget::GetUserDataKind(lua_State* L, int index)
{
    if (!lua_isuserdata(L, index))
        return UserDataKind::Unknown;
    UserDataHeader* header = (UserDataHeader*)lua_touserdata(L, index);
    return header->kind;
}

UserDataInfo InstanceTarget::GetUserDataInfo(lua_State* L, int index)
{
    IL2CPP_ASSERT(lua_isuserdata(L, index));
    UserDataHeader* header = (UserDataHeader*)lua_touserdata(L, index);
    if (header->kind == UserDataKind::ByVal)
    {
        ByValUserDataHeader* byValHeader = (ByValUserDataHeader*)header;
        return UserDataInfo{header->kind, byValHeader->klass};
    }
    else if (header->kind == UserDataKind::ByObj)
    {
        // Overload / facade must use declared view, not runtime obj->klass (e.g. cast to object).
        ZLuaObjectUserData* ud = (ZLuaObjectUserData*)header;
        return UserDataInfo{header->kind, ud->viewKlass};
    }
    else
    {
        LuaException::Throw("zlua: userdata must have a kind field");
    }
}

FnResolveMethodThis InstanceTarget::GetResolveMethodThisFunc(Il2CppClass* klass, bool isStatic, bool isByVal)
{
    if (isStatic)
    {
        return InstanceTarget::ResolveStaticMethodTarget;
    }
    bool isValueType = MetadataUtil::IsValueTypeClass(klass);
    if (isByVal)
    {
        if (isValueType)
        {
            return InstanceTarget::ResolveByValMethodTarget;
        }
        else
        {
            // luaL_error(L, "zlua: can't call method defined in parent class of struct, because needs to box the value. it is too expensive.");
            return (FnResolveMethodThis)InstanceTarget::ResolveBoxedByValMethodTarget;
        }
    }
    else
    {
        if (isValueType)
        {
            return InstanceTarget::ResolveByObjValueTypeMethodTarget;
        }
        else
        {
            return InstanceTarget::ResolveByObjReferenceMethodTarget;
        }
    }
}
} // namespace zlua
