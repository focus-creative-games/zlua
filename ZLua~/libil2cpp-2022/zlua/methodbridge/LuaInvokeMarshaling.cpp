#include "LuaInvokeMarshaling.h"

#include "../MetadataUtil.h"
#include "../StructOpaqueScope.h"
#include "../marshal/MarshalAsBytes.h"
#include "../marshal/MarshalAsOpaque.h"
#include "../marshal/MarshalAsUserData.h"
#include "../marshal/Marshaling.h"

#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/Array.h"

namespace zlua
{
    void LuaInvokeMarshaling::EnterCall()
    {
        StructOpaqueScope::EnterStandaloneCSharpToLua();
    }

    Il2CppClass* LuaInvokeMarshaling::ResolveClass(const char* assemblyName, const char* typeFullName)
    {
        if (assemblyName == nullptr || typeFullName == nullptr)
            return nullptr;

        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(assemblyName);
        if (assembly == nullptr)
            return nullptr;

        return MetadataUtil::ResolveType(assembly, typeFullName);
    }

    void LuaInvokeMarshaling::PushDefaultBool(lua_State* L, bool value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultInt32(lua_State* L, int32_t value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultUInt32(lua_State* L, uint32_t value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultInt64(lua_State* L, int64_t value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultUInt64(lua_State* L, uint64_t value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultFloat(lua_State* L, float value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultDouble(lua_State* L, double value)
    {
        detail::PushDefault(L, value);
    }

    void LuaInvokeMarshaling::PushDefaultString(lua_State* L, Il2CppString* value)
    {
        detail::PushDefault(L, value);
    }

    static Il2CppObject* BoxValue(Il2CppClass* klass, const void* valuePtr)
    {
        if (klass == nullptr)
            return nullptr;
        il2cpp::vm::Class::Init(klass);
        return il2cpp::vm::Object::Box(klass, const_cast<void*>(valuePtr));
    }

    void LuaInvokeMarshaling::PushInt32UserData(lua_State* L, int32_t value)
    {
        Il2CppClass* klass = MetadataUtil::ResolveCorlibType("System.Int32");
        Il2CppObject* boxed = BoxValue(klass, &value);
        MarshalAsUserData::Push(L, &klass->byval_arg, boxed);
    }

    void LuaInvokeMarshaling::PushStringUserData(lua_State* L, Il2CppString* value)
    {
        Il2CppClass* klass = il2cpp_defaults.string_class;
        MarshalAsUserData::Push(L, &klass->byval_arg, reinterpret_cast<Il2CppObject*>(value));
    }

    void LuaInvokeMarshaling::PushEnumUserData(lua_State* L, Il2CppClass* enumClass, int32_t value)
    {
        if (enumClass == nullptr)
            luaL_error(L, "zlua: missing enum class for LuaInvoke UserData push");

        Il2CppObject* boxed = BoxValue(enumClass, &value);
        MarshalAsUserData::Push(L, &enumClass->byval_arg, boxed);
    }

    void LuaInvokeMarshaling::PushByteArrayBytes(lua_State* L, Il2CppArray* value)
    {
        Il2CppClass* arrayClass = value != nullptr ? value->klass : MetadataUtil::ResolveCorlibType("System.Byte[]");
        if (arrayClass == nullptr)
            luaL_error(L, "zlua: missing byte[] class for LuaInvoke Bytes push");

        const Il2CppType* arrayType = &arrayClass->byval_arg;
        MarshalAsBytes::Push(L, arrayType, reinterpret_cast<Il2CppObject*>(value));
    }

    void LuaInvokeMarshaling::PushStructOpaque(lua_State* L, Il2CppClass* structClass, const void* valuePtr)
    {
        if (structClass == nullptr)
            luaL_error(L, "zlua: missing struct class for LuaInvoke Opaque push");

        Il2CppObject* boxed = BoxValue(structClass, valuePtr);
        MarshalAsOpaque::Push(L, &structClass->byval_arg, boxed);
    }

    static int32_t UnboxInt32(Il2CppObject* boxed)
    {
        if (boxed == nullptr)
            return 0;
        return *reinterpret_cast<int32_t*>(il2cpp::vm::Object::Unbox(boxed));
    }

    bool LuaInvokeMarshaling::PopDefaultBool(lua_State* L, int index)
    {
        return detail::PopDefault<bool>(L, index);
    }

    int32_t LuaInvokeMarshaling::PopDefaultInt32(lua_State* L, int index)
    {
        return detail::PopDefault<int32_t>(L, index);
    }

    Il2CppString* LuaInvokeMarshaling::PopDefaultString(lua_State* L, int index)
    {
        return detail::PopDefault<Il2CppString*>(L, index);
    }

    int32_t LuaInvokeMarshaling::PopInt32UserData(lua_State* L, int index)
    {
        Il2CppClass* klass = il2cpp_defaults.int32_class;
        int32_t boxed = 0;
        if (!MarshalAsUserData::TryPop(L, index, &klass->byval_arg, &boxed, sizeof(boxed)))
            return luaL_error(L, "zlua: LuaInvoke int UserData pop failed");
        return boxed;
    }

    Il2CppString* LuaInvokeMarshaling::PopStringUserData(lua_State* L, int index)
    {
        Il2CppClass* klass = il2cpp_defaults.string_class;
        Il2CppObject* boxed = nullptr;
        if (!MarshalAsUserData::TryPop(L, index, &klass->byval_arg, &boxed, sizeof(boxed)))
            luaL_error(L, "zlua: LuaInvoke string UserData pop failed");
        return reinterpret_cast<Il2CppString*>(boxed);
    }

    int32_t LuaInvokeMarshaling::PopEnumUserData(lua_State* L, int index, Il2CppClass* enumClass)
    {
        if (enumClass == nullptr)
            return luaL_error(L, "zlua: missing enum class for LuaInvoke UserData pop");

        int32_t boxed = 0;
        if (!MarshalAsUserData::TryPop(L, index, &enumClass->byval_arg, &boxed, sizeof(boxed)))
            return luaL_error(L, "zlua: LuaInvoke enum UserData pop failed");
        return boxed;
    }

    static Il2CppClass* s_byteArrayClass = nullptr;

    static Il2CppClass* GetByteArrayClass()
    {
        if (s_byteArrayClass == nullptr)
        {
            s_byteArrayClass = il2cpp::vm::Class::GetArrayClass(il2cpp_defaults.byte_class, 1);
        }
        return s_byteArrayClass;
    }

    Il2CppArray* LuaInvokeMarshaling::PopByteArrayBytes(lua_State* L, int index)
    {
        Il2CppClass* arrayClass = GetByteArrayClass();

        Il2CppObject* arrayObj = nullptr;
        if (!MarshalAsBytes::TryPop(L, index, &arrayClass->byval_arg, &arrayObj, sizeof(arrayObj)))
            luaL_error(L, "zlua: LuaInvoke byte[] Bytes pop failed");
        return reinterpret_cast<Il2CppArray*>(arrayObj);
    }
} // namespace zlua
