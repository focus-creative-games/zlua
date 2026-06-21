#include "LuaInteropManager.h"

#include "LuaEnv.h"
#include "MetadataUtil.h"
#include "MethodBridge.h"
#include "ObjectRegistry.h"

#include "vm/Class.h"
#include "vm/Object.h"
#include "il2cpp-tabledefs.h"

#include <cstring>
#include <string>
#include <unordered_map>

namespace novalua
{
    static std::unordered_map<Il2CppClass*, int> s_InstanceMtRefs;
    static int ResolveAssemblyTypeIndex(lua_State* L);

    static int PushLuaError(lua_State* L, const char* prefix, const char* detail)
    {
        std::string msg = std::string(prefix) + detail;
        return luaL_error(L, "%s", msg.c_str());
    }

    static Il2CppClass* GetClassFromTypeTable(lua_State* L, int index)
    {
        lua_getfield(L, index, "__assembly");
        const char* assemblyName = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, index, "__fullname");
        const char* typeName = lua_tostring(L, -1);
        lua_pop(L, 1);

        if (assemblyName == nullptr || typeName == nullptr)
            return nullptr;

        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(assemblyName);
        return MetadataUtil::ResolveType(assembly, typeName);
    }

    static void SetMethodField(lua_State* L, int tableIndex, const char* key);
    static void PushStaticMethodClosure(lua_State* L, const MethodInfo* method);
    static void PushInstanceMethodClosure(lua_State* L, const MethodInfo* method);
    static void PushInstanceMetatable(lua_State* L, Il2CppClass* klass);
    static void PushTypeTable(lua_State* L, Il2CppClass* klass);

    static int ResolveAssemblyTypeIndex(lua_State* L)
    {
        const char* typeName = luaL_checkstring(L, 2);
        if (typeName == nullptr || typeName[0] == '\0')
            return 0;

        lua_getfield(L, 1, typeName);
        if (lua_istable(L, -1))
            return 1;
        lua_pop(L, 1);

        lua_getfield(L, 1, "__assembly_name");
        const char* assemblyName = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (assemblyName == nullptr)
            return 0;

        const Il2CppAssembly* assembly = MetadataUtil::ResolveAssembly(assemblyName);
        Il2CppClass* klass = MetadataUtil::ResolveType(assembly, typeName);
        if (klass == nullptr)
            return 0;

        PushTypeTable(L, klass);
        lua_pushvalue(L, -1);
        lua_setfield(L, 1, typeName);
        return 1;
    }

    static int ResolveAssemblyIndex(lua_State* L)
    {
        const char* assemblyName = luaL_checkstring(L, 2);
        if (assemblyName == nullptr || assemblyName[0] == '\0')
            return 0;

        lua_getfield(L, 1, assemblyName);
        if (lua_istable(L, -1))
            return 1;
        lua_pop(L, 1);

        if (MetadataUtil::ResolveAssembly(assemblyName) == nullptr)
            return 0;

        lua_newtable(L);
        lua_pushstring(L, assemblyName);
        lua_setfield(L, -2, "__assembly_name");

        lua_newtable(L);
        lua_pushcfunction(L, ResolveAssemblyTypeIndex);
        lua_setfield(L, -2, "__index");
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        lua_setfield(L, 1, assemblyName);
        return 1;
    }

    static int NovaLuaTypeOf(lua_State* L)
    {
        if (!lua_istable(L, 1))
            return PushLuaError(L, "novalua.typeof expects a csharp type table: ", "");
        lua_pushvalue(L, 1);
        return 1;
    }

    static int NovaLuaCreateSignature(lua_State* L)
    {
        const char* methodName = luaL_checkstring(L, 1);
        const int top = lua_gettop(L);
        std::string signature = methodName;
        signature.push_back('(');
        for (int i = 2; i <= top; ++i)
        {
            if (i > 2)
                signature.push_back(',');
            Il2CppClass* klass = GetClassFromTypeTable(L, i);
            if (klass == nullptr)
                return PushLuaError(L, "novalua.create_signature arg is not a type: ", methodName);
            if (klass->namespaze != nullptr && klass->namespaze[0] != '\0')
            {
                signature += klass->namespaze;
                signature.push_back('.');
            }
            signature += klass->name;
        }
        signature.push_back(')');
        lua_pushstring(L, signature.c_str());
        return 1;
    }

    static int NovaLuaMakeGenericType(lua_State* L)
    {
        return PushLuaError(L, "novalua.make_generic_type is not implemented in Il2Cpp MVP: ", "");
    }

    static int InvokeStaticMethod(lua_State* L)
    {
        const MethodInfo* method = (const MethodInfo*)lua_touserdata(L, lua_upvalueindex(1));
        if (method == nullptr)
            return luaL_error(L, "novalua: invalid static method binding");

        if (method->parameters_count == 2)
            return MethodBridge::InvokeStaticIntIntInt(L, method, 1);

        return luaL_error(L, "novalua: unsupported static method signature");
    }

    static int InvokeInstanceMethod(lua_State* L)
    {
        const MethodInfo* method = (const MethodInfo*)lua_touserdata(L, lua_upvalueindex(1));
        Il2CppObject* instance = ObjectRegistry::GetObject(L, 1);
        if (method == nullptr || instance == nullptr)
            return luaL_error(L, "novalua: invalid instance method invocation");

        if (method->parameters_count == 0)
            return MethodBridge::InvokeInstanceInt(L, method, instance, 2);
        if (method->parameters_count == 1)
            return MethodBridge::InvokeInstanceVoidInt(L, method, instance, 2);

        return luaL_error(L, "novalua: unsupported instance method signature");
    }

    static int CreateTypeInstance(lua_State* L)
    {
        Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));
        if (klass == nullptr)
            return luaL_error(L, "novalua: invalid type binding");

        const int argCount = lua_gettop(L);
        const MethodInfo* ctor = MetadataUtil::FindConstructor(klass, argCount);
        if (argCount == 0)
        {
            if (ctor != nullptr)
            {
                const int ret = MethodBridge::InvokeConstructorNoArgs(L, ctor, klass);
                auto it = s_InstanceMtRefs.find(klass);
                if (it != s_InstanceMtRefs.end())
                {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
                    lua_setmetatable(L, -2);
                }
                return ret;
            }
            Il2CppObject* instance = il2cpp::vm::Object::New(klass);
            ObjectRegistry::PushObject(L, instance);
            auto it = s_InstanceMtRefs.find(klass);
            if (it != s_InstanceMtRefs.end())
            {
                lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
                lua_setmetatable(L, -2);
            }
            return 1;
        }

        return luaL_error(L, "novalua: unsupported constructor signature");
    }

    static int ReleaseUserData(lua_State* L)
    {
        ObjectRegistry::ReleaseObject(L, 1);
        return 0;
    }

    static void SetMethodField(lua_State* L, int tableIndex, const char* key)
    {
        const int absIndex = lua_absindex(L, tableIndex);
        lua_getfield(L, absIndex, key);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            lua_pushvalue(L, -1);
            lua_setfield(L, absIndex, key);
        }
        else
        {
            lua_pop(L, 1);
            lua_pop(L, 1);
        }
    }

    static void PushStaticMethodClosure(lua_State* L, const MethodInfo* method)
    {
        lua_pushlightuserdata(L, (void*)method);
        lua_pushcclosure(L, InvokeStaticMethod, 1);
    }

    static void PushInstanceMethodClosure(lua_State* L, const MethodInfo* method)
    {
        lua_pushlightuserdata(L, (void*)method);
        lua_pushcclosure(L, InvokeInstanceMethod, 1);
    }

    static void PushInstanceMetatable(lua_State* L, Il2CppClass* klass)
    {
        lua_newtable(L);
        const int mtIndex = lua_absindex(L, -1);

        il2cpp::vm::Class::Init(klass);
        for (uint16_t i = 0; i < klass->method_count; ++i)
        {
            const MethodInfo* method = klass->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if ((method->flags & METHOD_ATTRIBUTE_STATIC) != 0)
                continue;
            if (strcmp(method->name, ".ctor") == 0)
                continue;
            if (method->name[0] == 'g' && strncmp(method->name, "get_", 4) == 0)
                continue;
            if (method->name[0] == 's' && strncmp(method->name, "set_", 4) == 0)
                continue;

            PushInstanceMethodClosure(L, method);
            SetMethodField(L, mtIndex, method->name);
        }

        lua_pushvalue(L, -1);
        lua_setfield(L, mtIndex, "__index");

        lua_pushcfunction(L, ReleaseUserData);
        lua_setfield(L, mtIndex, "__gc");
    }

    static void PushTypeTable(lua_State* L, Il2CppClass* klass)
    {
        il2cpp::vm::Class::Init(klass);
        lua_newtable(L);
        const int typeTableIndex = lua_absindex(L, -1);

        const Il2CppImage* image = klass->image;
        const char* assemblyName = image != nullptr && image->name != nullptr ? image->name : "";
        std::string normalizedAssembly = assemblyName;
        const size_t dllPos = normalizedAssembly.rfind(".dll");
        if (dllPos != std::string::npos)
            normalizedAssembly.erase(dllPos);
        lua_pushstring(L, normalizedAssembly.c_str());
        lua_setfield(L, typeTableIndex, "__assembly");

        std::string fullName;
        if (klass->namespaze != nullptr && klass->namespaze[0] != '\0')
        {
            fullName = klass->namespaze;
            fullName.push_back('.');
            fullName += klass->name;
        }
        else
        {
            fullName = klass->name;
        }
        lua_pushstring(L, fullName.c_str());
        lua_setfield(L, typeTableIndex, "__fullname");
        lua_pushstring(L, klass->name);
        lua_setfield(L, typeTableIndex, "__name");

        for (uint16_t i = 0; i < klass->method_count; ++i)
        {
            const MethodInfo* method = klass->methods[i];
            if (method == nullptr || method->name == nullptr)
                continue;
            if ((method->flags & METHOD_ATTRIBUTE_STATIC) == 0)
                continue;
            if (strcmp(method->name, ".cctor") == 0)
                continue;

            PushStaticMethodClosure(L, method);
            SetMethodField(L, typeTableIndex, method->name);
        }

        PushInstanceMetatable(L, klass);
        lua_setfield(L, typeTableIndex, "__instance_mt");
        lua_getfield(L, typeTableIndex, "__instance_mt");
        s_InstanceMtRefs[klass] = luaL_ref(L, LUA_REGISTRYINDEX);

        lua_newtable(L);
        lua_pushlightuserdata(L, klass);
        lua_pushcclosure(L, CreateTypeInstance, 1);
        lua_setfield(L, -2, "__call");
        lua_setmetatable(L, typeTableIndex);

        lua_settop(L, typeTableIndex);
    }

    void LuaInteropManager::EnsureCSharpRoot()
    {
        lua_State* L = LuaEnv::GetState();
        const int oldTop = lua_gettop(L);

        lua_getglobal(L, "CSharp");
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_newtable(L);
            lua_pushcfunction(L, ResolveAssemblyIndex);
            lua_setfield(L, -2, "__index");
            lua_setmetatable(L, -2);
            lua_setglobal(L, "CSharp");
        }
        else
        {
            lua_pop(L, 1);
        }

        lua_settop(L, oldTop);
    }

    void LuaInteropManager::RegisterNovaLuaApi()
    {
        EnsureCSharpRoot();

        lua_State* L = LuaEnv::GetState();
        lua_pushcfunction(L, NovaLuaTypeOf);
        lua_setglobal(L, "__novalua_typeof");
        lua_pushcfunction(L, NovaLuaCreateSignature);
        lua_setglobal(L, "__novalua_create_signature");
        lua_pushcfunction(L, NovaLuaMakeGenericType);
        lua_setglobal(L, "__novalua_make_generic_type");
    }
}
