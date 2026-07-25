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
        return UserDataInfo{header->kind, ObjectMarshal::PopByObjThis(L, index)->klass};
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
