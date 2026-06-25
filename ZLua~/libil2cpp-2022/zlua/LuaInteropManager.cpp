#include "LuaInteropManager.h"

#include "FieldBridge.h"
#include "LuaEnv.h"
#include "MetaBinding.h"
#include "MetadataUtil.h"
#include "MethodBridge.h"
#include "ObjectRegistry.h"

#include "vm/Class.h"
#include "vm/Object.h"
#include "vm/Runtime.h"
#include "il2cpp-tabledefs.h"

#include <cstring>
#include <string>

namespace zlua
{
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

    static bool RawFieldIsTable(lua_State* L, int tableIndex, const char* key)
    {
        const int absIndex = lua_absindex(L, tableIndex);
        lua_pushstring(L, key);
        lua_rawget(L, absIndex);
        const bool isTable = lua_istable(L, -1);
        lua_pop(L, 1);
        return isTable;
    }

    static void PushTypeTable(lua_State* L, Il2CppClass* klass);

    static int ResolveAssemblyTypeIndex(lua_State* L)
    {
        const char* typeName = luaL_checkstring(L, 2);
        if (typeName == nullptr || typeName[0] == '\0')
            return 0;

        if (RawFieldIsTable(L, 1, typeName))
        {
            lua_pushstring(L, typeName);
            lua_rawget(L, 1);
            return 1;
        }

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

        if (RawFieldIsTable(L, 1, assemblyName))
        {
            lua_pushstring(L, assemblyName);
            lua_rawget(L, 1);
            return 1;
        }

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

    static int ZLuaTypeOf(lua_State* L)
    {
        if (!lua_istable(L, 1))
            return PushLuaError(L, "zlua.typeof expects a csharp type table: ", "");
        lua_pushvalue(L, 1);
        return 1;
    }

    static int ZLuaCreateSignature(lua_State* L)
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
                return PushLuaError(L, "zlua.create_signature arg is not a type: ", methodName);
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

    static int ZLuaMakeGenericType(lua_State* L)
    {
        return PushLuaError(L, "zlua.make_generic_type is not implemented in Il2Cpp MVP: ", "");
    }

    static bool AttachInstanceMetatable(lua_State* L, Il2CppClass* klass)
    {
        lua_pushstring(L, "__instance_mt");
        lua_rawget(L, 1);
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        lua_setmetatable(L, -2);
        return true;
    }

    static int CreateTypeInstance(lua_State* L)
    {
        Il2CppClass* klass = (Il2CppClass*)lua_touserdata(L, lua_upvalueindex(1));
        if (klass == nullptr)
            return luaL_error(L, "zlua: invalid type binding");

        const int argCount = lua_gettop(L) - 1;
        const MethodInfo* ctor = MetadataUtil::FindConstructor(klass, argCount);
        if (ctor == nullptr)
            return luaL_error(L, "zlua: no constructor found for type: %s", klass->name);
        if (argCount != 0)
            return luaL_error(L, "zlua: unsupported constructor signature");

        Il2CppObject* instance = il2cpp::vm::Object::New(klass);
        Il2CppException* exc = nullptr;
        il2cpp::vm::Runtime::Invoke(ctor, instance, nullptr, &exc);
        if (exc != nullptr)
            return luaL_error(L, "zlua: exception occurred while invoking constructor for type: %s", klass->name);

        ObjectRegistry::PushObject(L, instance);
        if (!AttachInstanceMetatable(L, klass))
            return luaL_error(L, "zlua: instance metatable missing for type: %s", klass->name);
        return 1;
    }

    static void PushTypeTable(lua_State* L, Il2CppClass* klass)
    {
        MetaBinding::EnsureBinding(L, klass);

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

        MetaBinding::PushInstanceMetatable(L, klass);
        lua_setfield(L, typeTableIndex, "__instance_mt");

        lua_newtable(L);
        lua_pushlightuserdata(L, klass);
        lua_pushcclosure(L, CreateTypeInstance, 1);
        lua_setfield(L, -2, "__call");

        MetaBinding::AttachStaticMetatable(L, klass, -1);
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

    void LuaInteropManager::RegisterZLuaApi()
    {
        EnsureCSharpRoot();

        lua_State* L = LuaEnv::GetState();
        lua_pushcfunction(L, ZLuaTypeOf);
        lua_setglobal(L, "__zlua_typeof");
        lua_pushcfunction(L, ZLuaCreateSignature);
        lua_setglobal(L, "__zlua_create_signature");
        lua_pushcfunction(L, ZLuaMakeGenericType);
        lua_setglobal(L, "__zlua_make_generic_type");
    }
}
