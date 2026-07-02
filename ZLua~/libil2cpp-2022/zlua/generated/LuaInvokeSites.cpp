#include "LuaInvokeSites.h"

#include "zlua/LuaEnv.h"
#include "zlua/StructOpaqueScope.h"
#include "zlua/methodbridge/LuaInvokeMarshaling.h"
#include "zlua/methodbridge/LuaInvokeRuntime.h"
#include "zlua/marshal/Marshaling.h"

#include "vm/InternalCalls.h"

struct __zlua_ic_ZLua_Tests_Fixtures_Point2D
{
    int32_t X;
    int32_t Y;
};

namespace zlua
{
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoByteArrayBytes = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoEnumUserData = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoIntDefault = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoIntUserData = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoStringUserData = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ExpectIntNotInteger = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ReceiveStructOpaque = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ReturnEnumUserData = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_SumByteArrayBytes = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoBool = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoByte = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoChar = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoDouble = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoFloat = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoInt = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoIntPtr = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoLong = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoSByte = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoShort = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoString = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoUInt = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoUIntPtr = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoULong = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoUShort = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_Noop = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnBool = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnByte = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnChar = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnDouble = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnEnum = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnFloat = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnInt = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnIntPtr = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnLong = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnSByte = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnShort = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnString = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnUInt = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnUIntPtr = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnULong = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnUShort = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_LuaInvokeMarshalProbe_SumInt = { LUA_NOREF, LUA_NOREF };
    LuaInvokeSite kSite_TC_LuaTestHost_RunAll = { LUA_NOREF, LUA_NOREF };

    void InitLuaInvokeSites()
    {
        kSite_LuaInvokeMarshalAsProbe_EchoByteArrayBytes.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_EchoByteArrayBytes.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "echo_byte_array_bytes");
        kSite_LuaInvokeMarshalAsProbe_EchoEnumUserData.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_EchoEnumUserData.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "echo_enum_userdata");
        kSite_LuaInvokeMarshalAsProbe_EchoIntDefault.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_EchoIntDefault.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "echo_int_default");
        kSite_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "echo_int_invalid_bytes");
        kSite_LuaInvokeMarshalAsProbe_EchoIntUserData.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_EchoIntUserData.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "echo_int_userdata");
        kSite_LuaInvokeMarshalAsProbe_EchoStringUserData.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_EchoStringUserData.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "echo_string_userdata");
        kSite_LuaInvokeMarshalAsProbe_ExpectIntNotInteger.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_ExpectIntNotInteger.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "expect_int_not_integer");
        kSite_LuaInvokeMarshalAsProbe_ReceiveStructOpaque.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_ReceiveStructOpaque.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "receive_struct_opaque");
        kSite_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "return_byte_array_bytes");
        kSite_LuaInvokeMarshalAsProbe_ReturnEnumUserData.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_ReturnEnumUserData.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "return_enum_userdata");
        kSite_LuaInvokeMarshalAsProbe_SumByteArrayBytes.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal_as");
        kSite_LuaInvokeMarshalAsProbe_SumByteArrayBytes.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal_as", "sum_byte_array_bytes");
        kSite_LuaInvokeMarshalProbe_EchoBool.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoBool.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_bool");
        kSite_LuaInvokeMarshalProbe_EchoByte.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoByte.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_byte");
        kSite_LuaInvokeMarshalProbe_EchoChar.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoChar.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_char");
        kSite_LuaInvokeMarshalProbe_EchoDouble.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoDouble.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_double");
        kSite_LuaInvokeMarshalProbe_EchoFloat.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoFloat.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_float");
        kSite_LuaInvokeMarshalProbe_EchoInt.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoInt.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_int");
        kSite_LuaInvokeMarshalProbe_EchoIntPtr.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoIntPtr.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_intptr");
        kSite_LuaInvokeMarshalProbe_EchoLong.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoLong.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_long");
        kSite_LuaInvokeMarshalProbe_EchoSByte.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoSByte.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_sbyte");
        kSite_LuaInvokeMarshalProbe_EchoShort.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoShort.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_short");
        kSite_LuaInvokeMarshalProbe_EchoString.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoString.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_string");
        kSite_LuaInvokeMarshalProbe_EchoUInt.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoUInt.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_uint");
        kSite_LuaInvokeMarshalProbe_EchoUIntPtr.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoUIntPtr.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_uintptr");
        kSite_LuaInvokeMarshalProbe_EchoULong.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoULong.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_ulong");
        kSite_LuaInvokeMarshalProbe_EchoUShort.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_EchoUShort.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "echo_ushort");
        kSite_LuaInvokeMarshalProbe_Noop.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_Noop.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "noop");
        kSite_LuaInvokeMarshalProbe_ReturnBool.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnBool.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_bool");
        kSite_LuaInvokeMarshalProbe_ReturnByte.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnByte.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_byte");
        kSite_LuaInvokeMarshalProbe_ReturnChar.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnChar.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_char");
        kSite_LuaInvokeMarshalProbe_ReturnDouble.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnDouble.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_double");
        kSite_LuaInvokeMarshalProbe_ReturnEnum.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnEnum.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_enum");
        kSite_LuaInvokeMarshalProbe_ReturnFloat.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnFloat.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_float");
        kSite_LuaInvokeMarshalProbe_ReturnInt.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnInt.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_int");
        kSite_LuaInvokeMarshalProbe_ReturnIntPtr.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnIntPtr.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_intptr");
        kSite_LuaInvokeMarshalProbe_ReturnLong.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnLong.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_long");
        kSite_LuaInvokeMarshalProbe_ReturnSByte.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnSByte.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_sbyte");
        kSite_LuaInvokeMarshalProbe_ReturnShort.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnShort.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_short");
        kSite_LuaInvokeMarshalProbe_ReturnString.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnString.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_string");
        kSite_LuaInvokeMarshalProbe_ReturnUInt.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnUInt.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_uint");
        kSite_LuaInvokeMarshalProbe_ReturnUIntPtr.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnUIntPtr.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_uintptr");
        kSite_LuaInvokeMarshalProbe_ReturnULong.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnULong.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_ulong");
        kSite_LuaInvokeMarshalProbe_ReturnUShort.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_ReturnUShort.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "return_ushort");
        kSite_LuaInvokeMarshalProbe_SumInt.moduleRef = LuaEnv::GetModuleRef("test_luainvoke.marshal");
        kSite_LuaInvokeMarshalProbe_SumInt.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("test_luainvoke.marshal", "sum_int");
        kSite_TC_LuaTestHost_RunAll.moduleRef = LuaEnv::GetModuleRef("luatest.run_all");
        kSite_TC_LuaTestHost_RunAll.funcRef = LuaEnv::GetOrCreateModuleFunctionRef("luatest.run_all", "run");
    }

    static Il2CppArray* IC_LuaInvokeMarshalAsProbe_EchoByteArrayBytes(Il2CppArray* value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_EchoByteArrayBytes.funcRef);
        LuaInvokeMarshaling::PushByteArrayBytes(L, value);
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        Il2CppArray* ret = LuaInvokeMarshaling::PopByteArrayBytes(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_EchoEnumUserData(int32_t value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_EchoEnumUserData.funcRef);
        LuaInvokeMarshaling::PushEnumUserData(L, LuaInvokeMarshaling::ResolveClass("ZLua.Tests", "ZLua.Tests.Fixtures.ColorKind"), static_cast<int32_t>(value));
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        int32_t ret = static_cast<int32_t>(LuaInvokeMarshaling::PopEnumUserData(L, -1, LuaInvokeMarshaling::ResolveClass("ZLua.Tests", "ZLua.Tests.Fixtures.ColorKind")));
        lua_settop(L, top);
        return ret;
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_EchoIntDefault(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalAsProbe_EchoIntDefault, value);
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes, value);
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_EchoIntUserData(int32_t value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_EchoIntUserData.funcRef);
        LuaInvokeMarshaling::PushInt32UserData(L, value);
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        int32_t ret = LuaInvokeMarshaling::PopInt32UserData(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static Il2CppString* IC_LuaInvokeMarshalAsProbe_EchoStringUserData(Il2CppString* value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_EchoStringUserData.funcRef);
        LuaInvokeMarshaling::PushStringUserData(L, value);
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        Il2CppString* ret = LuaInvokeMarshaling::PopStringUserData(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static Il2CppString* IC_LuaInvokeMarshalAsProbe_ExpectIntNotInteger(int32_t value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_ExpectIntNotInteger.funcRef);
        LuaInvokeMarshaling::PushInt32UserData(L, value);
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        Il2CppString* ret = LuaInvokeMarshaling::PopDefaultString(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_ReceiveStructOpaque(__zlua_ic_ZLua_Tests_Fixtures_Point2D value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_ReceiveStructOpaque.funcRef);
        LuaInvokeMarshaling::PushStructOpaque(L, LuaInvokeMarshaling::ResolveClass("ZLua.Tests", "ZLua.Tests.Fixtures.Point2D"), &value);
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        int32_t ret = LuaInvokeMarshaling::PopDefaultInt32(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static Il2CppArray* IC_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes()
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes.funcRef);
        Marshaling::LuaPCall(L, 0, 1, errfunc);
        Il2CppArray* ret = LuaInvokeMarshaling::PopByteArrayBytes(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_ReturnEnumUserData()
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_ReturnEnumUserData.funcRef);
        Marshaling::LuaPCall(L, 0, 1, errfunc);
        int32_t ret = static_cast<int32_t>(LuaInvokeMarshaling::PopEnumUserData(L, -1, LuaInvokeMarshaling::ResolveClass("ZLua.Tests", "ZLua.Tests.Fixtures.ColorKind")));
        lua_settop(L, top);
        return ret;
    }

    static int32_t IC_LuaInvokeMarshalAsProbe_SumByteArrayBytes(Il2CppArray* value)
    {
        lua_State* L = LuaEnv::GetState();
        const int top = lua_gettop(L);
        const int errfunc = LuaEnv::PushErrorHandler(L);
        LuaInvokeMarshaling::EnterCall();
        lua_rawgeti(L, LUA_REGISTRYINDEX, kSite_LuaInvokeMarshalAsProbe_SumByteArrayBytes.funcRef);
        LuaInvokeMarshaling::PushByteArrayBytes(L, value);
        Marshaling::LuaPCall(L, 1, 1, errfunc);
        int32_t ret = LuaInvokeMarshaling::PopDefaultInt32(L, -1);
        lua_settop(L, top);
        return ret;
    }

    static bool IC_LuaInvokeMarshalProbe_EchoBool(bool value)
    {
        return LuaInvokeRuntime::CallRet<bool>(kSite_LuaInvokeMarshalProbe_EchoBool, value);
    }

    static int32_t IC_LuaInvokeMarshalProbe_EchoByte(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_EchoByte, value);
    }

    static int32_t IC_LuaInvokeMarshalProbe_EchoChar(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_EchoChar, value);
    }

    static double IC_LuaInvokeMarshalProbe_EchoDouble(double value)
    {
        return LuaInvokeRuntime::CallRet<double>(kSite_LuaInvokeMarshalProbe_EchoDouble, value);
    }

    static float IC_LuaInvokeMarshalProbe_EchoFloat(float value)
    {
        return LuaInvokeRuntime::CallRet<float>(kSite_LuaInvokeMarshalProbe_EchoFloat, value);
    }

    static int32_t IC_LuaInvokeMarshalProbe_EchoInt(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_EchoInt, value);
    }

    static intptr_t IC_LuaInvokeMarshalProbe_EchoIntPtr(intptr_t value)
    {
        return LuaInvokeRuntime::CallRet<intptr_t>(kSite_LuaInvokeMarshalProbe_EchoIntPtr, value);
    }

    static int64_t IC_LuaInvokeMarshalProbe_EchoLong(int64_t value)
    {
        return LuaInvokeRuntime::CallRet<int64_t>(kSite_LuaInvokeMarshalProbe_EchoLong, value);
    }

    static int32_t IC_LuaInvokeMarshalProbe_EchoSByte(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_EchoSByte, value);
    }

    static int32_t IC_LuaInvokeMarshalProbe_EchoShort(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_EchoShort, value);
    }

    static Il2CppString* IC_LuaInvokeMarshalProbe_EchoString(Il2CppString* value)
    {
        return LuaInvokeRuntime::CallRet<Il2CppString*>(kSite_LuaInvokeMarshalProbe_EchoString, value);
    }

    static uint32_t IC_LuaInvokeMarshalProbe_EchoUInt(uint32_t value)
    {
        return LuaInvokeRuntime::CallRet<uint32_t>(kSite_LuaInvokeMarshalProbe_EchoUInt, value);
    }

    static uintptr_t IC_LuaInvokeMarshalProbe_EchoUIntPtr(uintptr_t value)
    {
        return LuaInvokeRuntime::CallRet<uintptr_t>(kSite_LuaInvokeMarshalProbe_EchoUIntPtr, value);
    }

    static uint64_t IC_LuaInvokeMarshalProbe_EchoULong(uint64_t value)
    {
        return LuaInvokeRuntime::CallRet<uint64_t>(kSite_LuaInvokeMarshalProbe_EchoULong, value);
    }

    static int32_t IC_LuaInvokeMarshalProbe_EchoUShort(int32_t value)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_EchoUShort, value);
    }

    static void IC_LuaInvokeMarshalProbe_Noop()
    {
        LuaInvokeRuntime::CallVoid(kSite_LuaInvokeMarshalProbe_Noop);
    }

    static bool IC_LuaInvokeMarshalProbe_ReturnBool()
    {
        return LuaInvokeRuntime::CallRet<bool>(kSite_LuaInvokeMarshalProbe_ReturnBool);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnByte()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnByte);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnChar()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnChar);
    }

    static double IC_LuaInvokeMarshalProbe_ReturnDouble()
    {
        return LuaInvokeRuntime::CallRet<double>(kSite_LuaInvokeMarshalProbe_ReturnDouble);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnEnum()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnEnum);
    }

    static float IC_LuaInvokeMarshalProbe_ReturnFloat()
    {
        return LuaInvokeRuntime::CallRet<float>(kSite_LuaInvokeMarshalProbe_ReturnFloat);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnInt()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnInt);
    }

    static intptr_t IC_LuaInvokeMarshalProbe_ReturnIntPtr()
    {
        return LuaInvokeRuntime::CallRet<intptr_t>(kSite_LuaInvokeMarshalProbe_ReturnIntPtr);
    }

    static int64_t IC_LuaInvokeMarshalProbe_ReturnLong()
    {
        return LuaInvokeRuntime::CallRet<int64_t>(kSite_LuaInvokeMarshalProbe_ReturnLong);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnSByte()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnSByte);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnShort()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnShort);
    }

    static Il2CppString* IC_LuaInvokeMarshalProbe_ReturnString()
    {
        return LuaInvokeRuntime::CallRet<Il2CppString*>(kSite_LuaInvokeMarshalProbe_ReturnString);
    }

    static uint32_t IC_LuaInvokeMarshalProbe_ReturnUInt()
    {
        return LuaInvokeRuntime::CallRet<uint32_t>(kSite_LuaInvokeMarshalProbe_ReturnUInt);
    }

    static uintptr_t IC_LuaInvokeMarshalProbe_ReturnUIntPtr()
    {
        return LuaInvokeRuntime::CallRet<uintptr_t>(kSite_LuaInvokeMarshalProbe_ReturnUIntPtr);
    }

    static uint64_t IC_LuaInvokeMarshalProbe_ReturnULong()
    {
        return LuaInvokeRuntime::CallRet<uint64_t>(kSite_LuaInvokeMarshalProbe_ReturnULong);
    }

    static int32_t IC_LuaInvokeMarshalProbe_ReturnUShort()
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_ReturnUShort);
    }

    static int32_t IC_LuaInvokeMarshalProbe_SumInt(int32_t a, int32_t b)
    {
        return LuaInvokeRuntime::CallRet<int32_t>(kSite_LuaInvokeMarshalProbe_SumInt, a, b);
    }

    static void IC_TC_LuaTestHost_RunAll()
    {
        LuaInvokeRuntime::CallVoid(kSite_TC_LuaTestHost_RunAll);
    }

    void RegisterGeneratedInternalCalls()
    {
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoByteArrayBytes", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_EchoByteArrayBytes);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoEnumUserData", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_EchoEnumUserData);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntDefault", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_EchoIntDefault);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidBytes", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntUserData", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_EchoIntUserData);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoStringUserData", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_EchoStringUserData);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ExpectIntNotInteger", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_ExpectIntNotInteger);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReceiveStructOpaque", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_ReceiveStructOpaque);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReturnByteArrayBytes", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReturnEnumUserData", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_ReturnEnumUserData);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::SumByteArrayBytes", (Il2CppMethodPointer)IC_LuaInvokeMarshalAsProbe_SumByteArrayBytes);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoBool", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoBool);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByte", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoByte);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoChar", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoChar);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoDouble", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoDouble);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoFloat", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoFloat);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoInt", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoInt);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntPtr", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoIntPtr);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoLong", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoLong);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoSByte", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoSByte);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoShort", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoShort);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoString", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoString);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUInt", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoUInt);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUIntPtr", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoUIntPtr);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoULong", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoULong);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUShort", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_EchoUShort);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::Noop", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_Noop);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnBool", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnBool);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByte", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnByte);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnChar", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnChar);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnDouble", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnDouble);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnEnum", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnEnum);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnFloat", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnFloat);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnInt", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnInt);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntPtr", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnIntPtr);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnLong", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnLong);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnSByte", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnSByte);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnShort", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnShort);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnString", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnString);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUInt", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnUInt);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUIntPtr", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnUIntPtr);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnULong", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnULong);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUShort", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_ReturnUShort);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumInt", (Il2CppMethodPointer)IC_LuaInvokeMarshalProbe_SumInt);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.TC_LuaTestHost::RunAll", (Il2CppMethodPointer)IC_TC_LuaTestHost_RunAll);
    }
}
