#include "ZLuaLib.h"

#include "LuaEnv.h"
#include "MetadataUtil.h"
#include "marshal/PrimitiveMarshal.h"
#include "marshal/ObjectMarshal.h"
#include "marshal/StructMarshal.h"
#include "marshal/ValueMarshaling.h"
#include "StructOpaqueScope.h"
#include "mt/MetaBinding.h"
#include "mt/TypeRegistry.h"
#include "delegate/LuaDelegateBinder.h"

#include "lua/lauxlib.h"
#include "vm/Array.h"
#include "vm/Class.h"
#include "vm/MetadataCache.h"
#include "vm/Object.h"

#include <string>
#include <vector>

namespace zlua
{
    static int PushLuaError(lua_State* L, const char* prefix, const char* detail)
    {
        std::string msg = std::string(prefix) + detail;
        return luaL_error(L, "%s", msg.c_str());
    }

    static Il2CppClass* ResolveTargetClass(lua_State* L, int index)
    {
        if (lua_istable(L, index))
            return TypeRegistry::GetClassFromTypeTable(L, index);

        if (lua_isuserdata(L, index))
        {
            Il2CppObject* obj = ObjectMarshal::Pop(L, index);
            return obj != nullptr ? obj->klass : nullptr;
        }

        return nullptr;
    }

    static bool TryGetConsecutiveTableLength(lua_State* L, int index, int& length, const char*& error)
    {
        length = 0;
        error = nullptr;
        if (!lua_istable(L, index))
        {
            error = "expected table";
            return false;
        }

        for (int i = 1; ; ++i)
        {
            lua_rawgeti(L, index, i);
            if (lua_isnil(L, -1))
            {
                lua_pop(L, 1);
                break;
            }
            lua_pop(L, 1);
            length = i;
        }
        return true;
    }

    static bool TryReadIntSequence(lua_State* L, int index, int expectedCount, std::vector<int32_t>& values, const char*& error)
    {
        values.clear();
        error = nullptr;
        if (!lua_istable(L, index))
        {
            error = "expected table";
            return false;
        }

        values.resize((size_t)expectedCount);
        for (int i = 0; i < expectedCount; ++i)
        {
            lua_rawgeti(L, index, i + 1);
            if (!lua_isinteger(L, -1))
            {
                lua_pop(L, 1);
                error = "table index must be integer";
                return false;
            }
            values[(size_t)i] = (int32_t)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        return true;
    }

    static int PushManagedInstance(lua_State* L, Il2CppObject* instance, Il2CppClass* klass)
    {
        if (instance == nullptr || klass == nullptr)
            return luaL_error(L, "zlua: invalid managed instance");

        ObjectMarshal::Push(L, instance);
        if (!ValueMarshaling::AttachInstanceMetatable(L, klass))
            return luaL_error(L, "zlua: instance metatable missing for type: %s", MetadataUtil::GetTypeFullName(klass));
        return 1;
    }

    static bool IsPrimitiveValueClass(Il2CppClass* klass)
    {
        if (klass == nullptr || klass->enumtype)
            return false;
        switch (klass->byval_arg.type)
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
            return true;
        default:
            return false;
        }
    }

    static bool UnboxReturnsLuaValue(Il2CppClass* klass)
    {
        return klass != nullptr && (klass->enumtype || IsPrimitiveValueClass(klass));
    }

    static int PushBoxedByObjInstance(lua_State* L, Il2CppClass* klass, Il2CppObject* boxed)
    {
        ObjectMarshal::Push(L, boxed);
        if (boxed == nullptr)
            return 1;

        if (ValueMarshaling::IsStructClass(klass) || ValueMarshaling::IsEnumClass(klass))
        {
            if (ValueMarshaling::AttachByObjInstanceMetatable(L, klass))
                return 1;
        }
        else if (ValueMarshaling::AttachInstanceMetatable(L, klass))
        {
            return 1;
        }

        TypeRegistry::PushInternedTypeTable(L, klass);
        const int typeTableIndex = lua_gettop(L);
        const int boxedIndex = typeTableIndex - 1;

        if (ValueMarshaling::IsStructClass(klass) || ValueMarshaling::IsEnumClass(klass))
            MetaBinding::PushByObjInstanceMetatable(L, klass, typeTableIndex);
        else
            MetaBinding::PushReferenceInstanceMetatable(L, klass, typeTableIndex);

        lua_setmetatable(L, boxedIndex);
        lua_settop(L, boxedIndex);
        return 1;
    }

    static int ZLuaBox(lua_State* L)
    {
        Il2CppClass* klass = MetadataUtil::ResolveTypeArg(L, 1);
        if (klass == nullptr)
            return luaL_error(L, "zlua.box expects typeArg as first argument");

        il2cpp::vm::Class::Init(klass);
        if (!klass->byval_arg.valuetype && !klass->enumtype)
            return luaL_error(L, "zlua.box expects value type, got: %s", MetadataUtil::GetTypeFullName(klass));

        const size_t payloadSize = ValueMarshaling::GetValueTypeInstanceSize(klass);
        std::vector<uint8_t> storage(payloadSize, 0);

        if (StructMarshal::IsByValUserData(L, 2))
        {
            if (!StructMarshal::PopValue(L, 2, klass, storage.data()))
                return luaL_error(L, "zlua.box value type mismatch for: %s", MetadataUtil::GetTypeFullName(klass));
        }
        else if (!ValueMarshaling::TryPop(L, 2, &klass->byval_arg, storage.data(), storage.size()))
        {
            return luaL_error(L, "zlua.box value mismatch for: %s", MetadataUtil::GetTypeFullName(klass));
        }

        Il2CppObject* boxed = il2cpp::vm::Object::Box(klass, storage.data());
        if (boxed == nullptr)
            return luaL_error(L, "zlua.box failed for: %s", MetadataUtil::GetTypeFullName(klass));

        return PushBoxedByObjInstance(L, klass, boxed);
    }

    static int ZLuaUnbox(lua_State* L)
    {
        if (StructMarshal::IsByValUserData(L, 1))
            return luaL_error(L, "zlua.unbox expects ByObjUserData");

        Il2CppObject* obj = ObjectMarshal::Pop(L, 1);
        if (obj == nullptr)
            return luaL_error(L, "zlua.unbox expects ByObjUserData");

        Il2CppClass* klass = obj->klass;
        il2cpp::vm::Class::Init(klass);
        if (!klass->byval_arg.valuetype && !klass->enumtype)
            return luaL_error(L, "zlua.unbox expects boxed value type");

        if (UnboxReturnsLuaValue(klass))
        {
            const Il2CppType* valueType = klass->enumtype
                ? il2cpp::vm::Class::GetEnumBaseType(klass)
                : &klass->byval_arg;
            if (valueType == nullptr)
                return luaL_error(L, "zlua.unbox enum base type missing");
            const int pushed = PrimitiveMarshal::PushBoxedReturn(L, valueType, il2cpp::vm::Object::Unbox(obj));
            if (pushed == 0)
                return luaL_error(L, "zlua.unbox unsupported primitive type");
            return pushed;
        }

        ValueMarshaling::PushStructInstance(L, klass, obj);
        return 1;
    }

    static int CreateMdArrayInstance(
        lua_State* L,
        Il2CppClass* elementClass,
        Il2CppClass* arrayClass,
        int rank,
        int lowboundsIndex,
        int sizesIndex)
    {
        const char* error = nullptr;
        int lowboundsLength = 0;
        int sizesLength = 0;
        if (!TryGetConsecutiveTableLength(L, lowboundsIndex, lowboundsLength, error))
            return luaL_error(L, "zlua.new_mdarray_* lowbounds: %s", error != nullptr ? error : "invalid table");
        if (!TryGetConsecutiveTableLength(L, sizesIndex, sizesLength, error))
            return luaL_error(L, "zlua.new_mdarray_* sizes: %s", error != nullptr ? error : "invalid table");
        if (lowboundsLength != rank)
            return luaL_error(L, "zlua.new_mdarray_* lowbounds length must be %d", rank);
        if (sizesLength != rank)
            return luaL_error(L, "zlua.new_mdarray_* sizes length must be %d", rank);

        std::vector<int32_t> lowerBounds;
        std::vector<int32_t> sizes;
        if (!TryReadIntSequence(L, lowboundsIndex, rank, lowerBounds, error))
            return luaL_error(L, "zlua.new_mdarray_* lowbounds: %s", error != nullptr ? error : "invalid table");
        if (!TryReadIntSequence(L, sizesIndex, rank, sizes, error))
            return luaL_error(L, "zlua.new_mdarray_* sizes: %s", error != nullptr ? error : "invalid table");

        for (int32_t size : sizes)
        {
            if (size < 0)
                return luaL_error(L, "zlua.new_mdarray_* sizes must be >= 0");
        }

        std::vector<il2cpp_array_size_t> bounds((size_t)rank);
        std::vector<il2cpp_array_size_t> lengths((size_t)rank);
        for (int i = 0; i < rank; ++i)
        {
            bounds[(size_t)i] = (il2cpp_array_size_t)lowerBounds[(size_t)i];
            lengths[(size_t)i] = (il2cpp_array_size_t)sizes[(size_t)i];
        }

        Il2CppArray* array = il2cpp::vm::Array::NewFull(arrayClass, lengths.data(), bounds.data());
        if (array == nullptr)
            return luaL_error(L, "zlua: failed to create mdarray");

        return PushManagedInstance(L, (Il2CppObject*)array, arrayClass);
    }

    static int ZLuaTypeOf(lua_State* L)
    {
        if (!lua_istable(L, 1))
            return PushLuaError(L, "zlua.typeof expects a csharp type table: ", "");
        lua_pushvalue(L, 1);
        return 1;
    }

    static int ZLuaCreateSignature(lua_State* L)
    {
        const int top = lua_gettop(L);
        std::string signature = "(";
        for (int i = 1; i <= top; ++i)
        {
            if (i > 1)
                signature.push_back(',');
            Il2CppClass* klass = MetadataUtil::ResolveTypeArg(L, i);
            if (klass == nullptr)
                return PushLuaError(L, "zlua.signature arg", " is not a type");
            signature += MetadataUtil::GetSignatureTypeName(klass);
        }
        signature.push_back(')');
        lua_pushstring(L, signature.c_str());
        return 1;
    }

    static int ZLuaMakeGenericType(lua_State* L)
    {
        if (!lua_istable(L, 1))
            return luaL_error(L, "zlua.make_generic_type expects generic type table as first arg");

        Il2CppClass* genericDef = TypeRegistry::GetClassFromTypeTable(L, 1);
        if (genericDef == nullptr)
            return luaL_error(L, "zlua.make_generic_type expects generic type table as first arg");

        if (!il2cpp::vm::Class::IsGenericTypeDefinition(genericDef))
            return luaL_error(L, "type is not a generic definition: %s", MetadataUtil::GetTypeFullName(genericDef));

        const Il2CppMetadataGenericContainerHandle container = il2cpp::vm::Class::GetGenericContainer(genericDef);
        const uint32_t expectedCount = il2cpp::vm::MetadataCache::GetGenericContainerCount(container);
        const int argCount = lua_gettop(L) - 1;
        if ((uint32_t)argCount != expectedCount)
            return luaL_error(L, "generic arg count mismatch: expected %u, got %d", expectedCount, argCount);

        std::vector<const Il2CppType*> typeArgs((size_t)argCount);
        for (int i = 0; i < argCount; ++i)
        {
            Il2CppClass* argClass = MetadataUtil::ResolveTypeArg(L, i + 2);
            if (argClass == nullptr)
                return luaL_error(L, "generic arg %d is not a type", i + 1);
            typeArgs[(size_t)i] = &argClass->byval_arg;
        }

        Il2CppClass* closedType = il2cpp::vm::MetadataCache::GetGenericInstanceType(
            genericDef,
            typeArgs.data(),
            (uint32_t)argCount);
        if (closedType == nullptr)
            return luaL_error(L, "zlua make_generic_type error: failed to construct closed type");

        TypeRegistry::PushInternedTypeTable(L, closedType);
        return 1;
    }

    static int ZLuaMakeSzArrayType(lua_State* L)
    {
        Il2CppClass* elementClass = MetadataUtil::ResolveTypeArg(L, 1);
        if (elementClass == nullptr)
            return luaL_error(L, "zlua.make_szarray_type expects element type");

        Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, 1);
        if (arrayClass == nullptr)
            return luaL_error(L, "zlua.make_szarray_type failed");

        TypeRegistry::PushInternedTypeTable(L, arrayClass);
        return 1;
    }

    static int ZLuaMakeMdArrayType(lua_State* L)
    {
        Il2CppClass* elementClass = MetadataUtil::ResolveTypeArg(L, 1);
        if (elementClass == nullptr)
            return luaL_error(L, "zlua.make_mdarray_type expects element type");

        const int rank = (int)luaL_checkinteger(L, 2);
        if (rank < 1)
            return luaL_error(L, "zlua.make_mdarray_type rank must be >= 1");

        Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, (uint32_t)rank);
        if (arrayClass == nullptr)
            return luaL_error(L, "zlua.make_mdarray_type failed");

        TypeRegistry::PushInternedTypeTable(L, arrayClass);
        return 1;
    }

    static int ZLuaNewSzArrayByElementType(lua_State* L)
    {
        Il2CppClass* elementClass = MetadataUtil::ResolveTypeArg(L, 1);
        if (elementClass == nullptr)
            return luaL_error(L, "zlua.new_szarray_by_element_type expects element type");

        if (!lua_isinteger(L, 2))
            return luaL_error(L, "zlua.new_szarray_by_element_type expects integer length");

        const lua_Integer length = lua_tointeger(L, 2);
        if (length < 0)
            return luaL_error(L, "zlua.new_szarray_by_element_type length must be >= 0");

        Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, 1);
        Il2CppArray* array = il2cpp::vm::Array::NewSpecific(arrayClass, (il2cpp_array_size_t)length);
        if (array == nullptr)
            return luaL_error(L, "zlua: failed to create szarray");

        return PushManagedInstance(L, (Il2CppObject*)array, arrayClass);
    }

    static int ZLuaNewSzArrayBySzArrayType(lua_State* L)
    {
        Il2CppClass* arrayClass = MetadataUtil::ResolveTypeArg(L, 1);
        if (arrayClass == nullptr || !IL2CPP_CLASS_IS_ARRAY(arrayClass))
            return luaL_error(L, "zlua.new_szarray_by_szarray_type expects szarray type table");
        if (arrayClass->rank != 1)
            return luaL_error(L, "zlua.new_szarray_by_szarray_type expects rank-1 array type");

        if (!lua_isinteger(L, 2))
            return luaL_error(L, "zlua.new_szarray_by_szarray_type expects integer length");

        const lua_Integer length = lua_tointeger(L, 2);
        if (length < 0)
            return luaL_error(L, "zlua.new_szarray_by_szarray_type length must be >= 0");

        Il2CppArray* array = il2cpp::vm::Array::NewSpecific(arrayClass, (il2cpp_array_size_t)length);
        if (array == nullptr)
            return luaL_error(L, "zlua: failed to create szarray");

        return PushManagedInstance(L, (Il2CppObject*)array, arrayClass);
    }

    static int ZLuaNewMdArrayByMdArrayType(lua_State* L)
    {
        Il2CppClass* arrayClass = MetadataUtil::ResolveTypeArg(L, 1);
        if (arrayClass == nullptr || !IL2CPP_CLASS_IS_ARRAY(arrayClass))
            return luaL_error(L, "zlua.new_mdarray_by_mdarray_type expects mdarray type table");

        Il2CppClass* elementClass = arrayClass->element_class;
        if (elementClass == nullptr)
            return luaL_error(L, "zlua.new_mdarray_by_mdarray_type expects array type");

        return CreateMdArrayInstance(L, elementClass, arrayClass, (int)arrayClass->rank, 2, 3);
    }

    static int ZLuaNewMdArrayBySpec(lua_State* L)
    {
        Il2CppClass* elementClass = MetadataUtil::ResolveTypeArg(L, 1);
        if (elementClass == nullptr)
            return luaL_error(L, "zlua.new_mdarray_by_spec expects element type");

        const char* error = nullptr;
        int rank = 0;
        if (!TryGetConsecutiveTableLength(L, 3, rank, error))
            return luaL_error(L, "zlua.new_mdarray_by_spec sizes: %s", error != nullptr ? error : "invalid table");
        if (rank < 1)
            return luaL_error(L, "zlua.new_mdarray_by_spec rank must be >= 1");

        Il2CppClass* arrayClass = il2cpp::vm::Class::GetArrayClass(elementClass, (uint32_t)rank);
        return CreateMdArrayInstance(L, elementClass, arrayClass, rank, 2, 3);
    }

    static int ZLuaGetMethod(lua_State* L)
    {
        if (lua_gettop(L) < 4)
            return luaL_error(L, "zlua.get_method expects (target, methodName, signature, is_static)");

        Il2CppClass* targetClass = ResolveTargetClass(L, 1);
        if (targetClass == nullptr)
            return luaL_error(L, "zlua.get_method expects userdata or type table as target");

        const char* methodName = luaL_checkstring(L, 2);
        const char* signature = luaL_checkstring(L, 3);
        const bool isStatic = lua_toboolean(L, 4) != 0;

        const MethodInfo* method = MetadataUtil::FindMethodByParameterSignature(
            targetClass,
            methodName,
            signature,
            isStatic);
        if (method == nullptr)
        {
            return luaL_error(
                L,
                "zlua: no overload for %s.%s matching %s",
                MetadataUtil::GetSignatureTypeName(targetClass).c_str(),
                methodName,
                signature);
        }

        return MetaBinding::PushMethodClosure(L, method, isStatic);
    }

    static int ZLuaRegisterMethod(lua_State* L)
    {
        if (lua_gettop(L) < 3)
            return luaL_error(L, "zlua.register_method expects (static_class_mt_or_obj, aliasName, methodOrClosure)");

        const char* aliasName = luaL_checkstring(L, 2);
        if (aliasName == nullptr || aliasName[0] == '\0')
            return luaL_error(L, "zlua.register_method expects alias name");

        if (!lua_isfunction(L, 3))
            return luaL_error(L, "zlua.register_method expects callable closure");

        bool isStatic = false;
        Il2CppClass* klass = nullptr;
        if (lua_istable(L, 1))
        {
            klass = TypeRegistry::GetClassFromTypeTable(L, 1);
            isStatic = true;
            if (klass == nullptr)
                return luaL_error(L, "zlua.register_method expects type table as first argument");
        }
        else if (lua_isuserdata(L, 1))
        {
            Il2CppObject* obj = ObjectMarshal::Pop(L, 1);
            if (obj == nullptr)
                return luaL_error(L, "zlua.register_method expects userdata instance");
            klass = obj->klass;
            isStatic = false;
        }
        else
        {
            return luaL_error(L, "zlua.register_method expects type table or userdata instance");
        }

        if (!MetaBinding::TryRegisterMethodAlias(L, klass, isStatic, aliasName, 3))
            return luaL_error(L, "zlua: method alias already exists: %s", aliasName);

        return 0;
    }

    static int ZLuaToDelegate(lua_State* L)
    {
        if (!lua_isfunction(L, 1))
            return luaL_error(L, "zlua.to_delegate expects Lua function");

        Il2CppClass* delegateClass = MetadataUtil::ResolveTypeArg(L, 2);
        if (delegateClass == nullptr)
            return luaL_error(L, "zlua.to_delegate expects closed delegate type");

        if (!MetadataUtil::IsDelegateClass(delegateClass))
            return luaL_error(L, "zlua.to_delegate expects delegate type");

        Il2CppDelegate* delegate = LuaDelegateBinder::CreateFromStack(L, 1, delegateClass);
        if (delegate == nullptr)
            return luaL_error(L, "unsupported delegate signature for Lua callback");

        return PushManagedInstance(L, reinterpret_cast<Il2CppObject*>(delegate), delegateClass);
    }

    static int ZLuaToUserData(lua_State* L)
    {
        if (lua_type(L, 1) != LUA_TLIGHTUSERDATA)
            return luaL_error(L, "zlua.to_user_data: opaque must be lightuserdata");

        const intptr_t handleId = reinterpret_cast<intptr_t>(lua_touserdata(L, 1));
        Il2CppClass* klass = nullptr;
        const uint8_t* payload = nullptr;
        size_t payloadSize = 0;
        if (!StructOpaqueScope::TryResolveEntry(handleId, &klass, &payload, &payloadSize))
            return luaL_error(L, "zlua.to_user_data: opaque handle is invalid or expired");

        if (klass == nullptr)
            return luaL_error(L, "zlua.to_user_data: opaque handle is invalid or expired");

        il2cpp::vm::Class::Init(klass);
        if (ValueMarshaling::IsStructClass(klass))
        {
            std::vector<uint8_t> copy(payloadSize);
            if (payloadSize > 0 && payload != nullptr)
                std::memcpy(copy.data(), payload, payloadSize);
            StructMarshal::PushValue(L, copy.data(), klass);
            return 1;
        }

        return luaL_error(L, "zlua.to_user_data: unsupported opaque type %s", MetadataUtil::GetTypeFullName(klass));
    }

    void ZLuaLib::RegisterGlobals()
    {
        lua_State* L = LuaEnv::GetState();
        lua_pushcfunction(L, ZLuaTypeOf);
        lua_setglobal(L, "__zlua_typeof");
        lua_pushcfunction(L, ZLuaCreateSignature);
        lua_setglobal(L, "__zlua_create_signature");
        lua_pushcfunction(L, ZLuaMakeGenericType);
        lua_setglobal(L, "__zlua_make_generic_type");
        lua_pushcfunction(L, ZLuaMakeSzArrayType);
        lua_setglobal(L, "__zlua_make_szarray_type");
        lua_pushcfunction(L, ZLuaMakeMdArrayType);
        lua_setglobal(L, "__zlua_make_mdarray_type");
        lua_pushcfunction(L, ZLuaNewSzArrayByElementType);
        lua_setglobal(L, "__zlua_new_szarray_by_element_type");
        lua_pushcfunction(L, ZLuaNewSzArrayBySzArrayType);
        lua_setglobal(L, "__zlua_new_szarray_by_szarray_type");
        lua_pushcfunction(L, ZLuaNewMdArrayByMdArrayType);
        lua_setglobal(L, "__zlua_new_mdarray_by_mdarray_type");
        lua_pushcfunction(L, ZLuaNewMdArrayBySpec);
        lua_setglobal(L, "__zlua_new_mdarray_by_spec");
        lua_pushcfunction(L, ZLuaGetMethod);
        lua_setglobal(L, "__zlua_get_method");
        lua_pushcfunction(L, ZLuaRegisterMethod);
        lua_setglobal(L, "__zlua_register_method");
        lua_pushcfunction(L, ZLuaBox);
        lua_setglobal(L, "__zlua_box");
        lua_pushcfunction(L, ZLuaUnbox);
        lua_setglobal(L, "__zlua_unbox");
        lua_pushcfunction(L, ZLuaToDelegate);
        lua_setglobal(L, "__zlua_to_delegate");
        lua_pushcfunction(L, ZLuaToUserData);
        lua_setglobal(L, "__zlua_to_user_data");
    }
}
