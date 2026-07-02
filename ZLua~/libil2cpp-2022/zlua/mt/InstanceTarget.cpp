#include "InstanceTarget.h"

#include "../marshal/ObjectMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/ValueMarshaling.h"

#include "vm/Class.h"
#include "vm/Object.h"

namespace zlua
{
    UserDataKind InstanceTarget::GetUserDataKind(lua_State* L, int index)
    {
        if (!lua_isuserdata(L, index))
            return UserDataKind::Unknown;

        if (StructMarshal::IsByValUserData(L, index))
            return UserDataKind::ByVal;

        if (lua_getmetatable(L, index) != 0)
        {
            lua_getfield(L, -1, "__zlua_ud_kind");
            const bool isByObj = lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), "byobj") == 0;
            lua_pop(L, 2);
            if (isByObj)
                return UserDataKind::ByObj;
        }

        if (ObjectMarshal::Pop(L, index) != nullptr)
            return UserDataKind::ByObj;

        return UserDataKind::Unknown;
    }

    static bool IsValueTypeMethodClass(const MethodInfo* method)
    {
        if (method == nullptr || method->klass == nullptr)
            return false;
        return method->klass->byval_arg.valuetype && !method->klass->enumtype;
    }

    void* InstanceTarget::ResolveMethodTarget(lua_State* L, int index, const MethodInfo* method)
    {
        if (method == nullptr)
            return nullptr;

        if (StructMarshal::IsByValUserData(L, index))
        {
            ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
            if (header == nullptr)
                return nullptr;

            void* payload = header->Payload();
            if (IsValueTypeMethodClass(method))
                return payload;

            Il2CppObject* boxed = il2cpp::vm::Object::Box(header->klass, payload);
            return boxed;
        }

        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return nullptr;

        if (IsValueTypeMethodClass(method))
            return reinterpret_cast<uint8_t*>(obj) + sizeof(Il2CppObject);

        return obj;
    }

    void* InstanceTarget::ResolveFieldAddress(lua_State* L, int index, int32_t payloadRelativeOffset, const FieldInfo* field)
    {
        if (field == nullptr)
            return nullptr;

        if (StructMarshal::IsByValUserData(L, index))
        {
            ByValUserDataHeader* header = StructMarshal::GetByValHeader(L, index);
            if (header == nullptr)
                return nullptr;
            return header->Payload() + payloadRelativeOffset;
        }

        Il2CppObject* obj = ObjectMarshal::Pop(L, index);
        if (obj == nullptr)
            return nullptr;

        if (field->parent != nullptr && field->parent->byval_arg.valuetype)
            return reinterpret_cast<uint8_t*>(obj) + sizeof(Il2CppObject) + payloadRelativeOffset;

        return reinterpret_cast<uint8_t*>(obj) + payloadRelativeOffset;
    }
}
