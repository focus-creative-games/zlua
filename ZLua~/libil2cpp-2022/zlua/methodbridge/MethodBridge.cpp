#include "MethodBridge.h"

#include "../MetadataUtil.h"
#include "../StructOpaqueScope.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/ValueMarshaling.h"

#include "vm/Object.h"
#include "vm/Runtime.h"
#include "vm/Class.h"
#include "vm/Array.h"
#include "vm/Type.h"

#include <cstring>
#include <vector>

namespace zlua
{
    static bool TryBuildParamsArray(
        lua_State* L,
        const MethodInfo* method,
        int paramsIndex,
        int argStart,
        int argCount,
        Il2CppArray** outArray)
    {
        *outArray = nullptr;
        const Il2CppType* paramsType = method->parameters[paramsIndex];
        Il2CppClass* arrayClass = il2cpp::vm::Class::FromIl2CppType(paramsType);
        if (arrayClass == nullptr || !IL2CPP_CLASS_IS_ARRAY(arrayClass))
            return false;

        Il2CppClass* elementClass = arrayClass->element_class;
        if (elementClass == nullptr)
            return false;

        const int fixedCount = paramsIndex;
        const int varCount = argCount - fixedCount;
        if (varCount == 1)
        {
            Il2CppObject* arrayObj = nullptr;
            if (ValueMarshaling::TryPop(L, argStart + fixedCount, paramsType, &arrayObj, sizeof(Il2CppObject*)))
            {
                *outArray = (Il2CppArray*)arrayObj;
                return true;
            }
        }

        Il2CppArray* array = il2cpp::vm::Array::NewSpecific(arrayClass, (il2cpp_array_size_t)varCount);
        if (array == nullptr)
            return false;

        const size_t elementSize = (size_t)il2cpp::vm::Array::GetElementSize(arrayClass);
        char* elementAddress = il2cpp::vm::Array::GetFirstElementAddress(array);
        for (int i = 0; i < varCount; ++i)
        {
            if (!ValueMarshaling::TryPop(
                    L,
                    argStart + fixedCount + i,
                    &elementClass->byval_arg,
                    elementAddress + (size_t)i * elementSize,
                    elementSize))
            {
                return false;
            }
        }

        *outArray = array;
        return true;
    }

    static int InvokeMethodImpl(lua_State* L, const MethodInfo* method, void* target, int argStart)
    {
        if (method == nullptr)
            return luaL_error(L, "zlua: invalid method");

        const int luaArgCount = lua_gettop(L) - argStart + 1;
        const int paramsIndex = ValueMarshaling::FindParamsParameterIndex(method);
        const int paramCount = method->parameters_count;
        std::vector<void*> params((size_t)paramCount, nullptr);
        std::vector<std::vector<uint8_t>> storage;
        storage.reserve((size_t)paramCount);
        Il2CppArray* paramsArray = nullptr;

        for (int i = 0; i < paramCount; ++i)
        {
            if (i == paramsIndex)
            {
                if (!TryBuildParamsArray(L, method, paramsIndex, argStart, luaArgCount, &paramsArray))
                    return luaL_error(L, "zlua: argument mismatch for %s", method->name);
                params[(size_t)i] = paramsArray;
                continue;
            }

            const Il2CppType* paramType = method->parameters[i];
            Il2CppClass* paramClass = il2cpp::vm::Class::FromIl2CppType(paramType, false);
            size_t sz = sizeof(void*);
            if (paramClass != nullptr && paramClass->byval_arg.valuetype)
                sz = ValueMarshaling::GetValueTypeInstanceSize(paramClass);
            else if (il2cpp::vm::Type::IsReference(paramType))
                sz = sizeof(Il2CppObject*);
            else if (paramType->type == IL2CPP_TYPE_R8)
                sz = sizeof(double);
            else if (paramType->type == IL2CPP_TYPE_R4)
                sz = sizeof(float);
            else
                sz = sizeof(int64_t);

            storage.emplace_back(sz);
            if (!ValueMarshaling::TryPop(
                    L,
                    argStart + i,
                    paramType,
                    storage.back().data(),
                    storage.back().size(),
                    method,
                    i))
                return luaL_error(L, "zlua: argument mismatch for %s", method->name);

            void* dataPtr = storage.back().data();
            params[(size_t)i] = il2cpp::vm::Type::IsReference(paramType) || il2cpp::vm::Type::IsPointerType(paramType) ? *(Il2CppObject**)dataPtr : dataPtr;
        }

        Il2CppException* exc = nullptr;
        Il2CppObject* ret = il2cpp::vm::Runtime::Invoke(method, target, paramCount > 0 ? params.data() : nullptr, &exc);
        if (exc != nullptr)
            return luaL_error(L, "zlua: exception in %s", method->name);

        return ValueMarshaling::PushReturn(L, method->return_type, ret, method);
    }

    int MethodBridge::InvokeMethod(lua_State* L, const MethodInfo* method, void* target, int argStart)
    {
        if (method == nullptr)
            return luaL_error(L, "zlua: invalid method");

        StructOpaqueScope::EnterLuaToCSharp();
        const int result = InvokeMethodImpl(L, method, target, argStart);
        StructOpaqueScope::LeaveLuaToCSharp();
        return result;
    }

    int MethodBridge::InvokeConstructor(lua_State* L, const MethodInfo* ctor, Il2CppClass* klass, int argStart, int argCount)
    {
        if (klass == nullptr)
            return luaL_error(L, "zlua: invalid type");

        StructOpaqueScope::EnterLuaToCSharp();
        Il2CppObject* instance = il2cpp::vm::Object::New(klass);
        if (instance == nullptr)
        {
            StructOpaqueScope::LeaveLuaToCSharp();
            return luaL_error(L, "zlua: failed to create instance");
        }

        void* ctorTarget = instance;
        if (ValueMarshaling::IsStructClass(klass))
            ctorTarget = il2cpp::vm::Object::Unbox(instance);

        int invokeResult = 0;
        if (ctor != nullptr)
        {
            invokeResult = InvokeMethodImpl(L, ctor, ctorTarget, argStart);
            if (invokeResult != 0)
            {
                StructOpaqueScope::LeaveLuaToCSharp();
                return invokeResult;
            }
        }
        else if (argCount != 0)
        {
            StructOpaqueScope::LeaveLuaToCSharp();
            return luaL_error(L, "zlua: no constructor found for type: %s", klass->name);
        }

        if (ValueMarshaling::IsStructClass(klass))
            ValueMarshaling::PushStructInstance(L, klass, instance);
        else
        {
            ObjectMarshal::Push(L, instance);
            if (!ValueMarshaling::AttachInstanceMetatable(L, klass))
            {
                StructOpaqueScope::LeaveLuaToCSharp();
                return luaL_error(L, "zlua: instance metatable missing for type: %s", klass->name);
            }
        }

        StructOpaqueScope::LeaveLuaToCSharp();
        return 1;
    }
}
