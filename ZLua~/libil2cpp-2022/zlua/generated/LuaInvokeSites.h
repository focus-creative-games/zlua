#pragma once

#include "zlua/methodbridge/LuaInvokeRuntime.h"

namespace zlua
{
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoByteArrayBytes;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoEnumUserData;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoIntDefault;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoIntUserData;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_EchoStringUserData;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ExpectIntNotInteger;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ReceiveStructOpaque;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_ReturnEnumUserData;
    extern LuaInvokeSite kSite_LuaInvokeMarshalAsProbe_SumByteArrayBytes;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoBool;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoByte;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoChar;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoDouble;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoFloat;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoInt;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoIntPtr;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoLong;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoSByte;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoShort;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoString;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoUInt;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoUIntPtr;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoULong;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_EchoUShort;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_Noop;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnBool;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnByte;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnChar;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnDouble;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnEnum;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnFloat;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnInt;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnIntPtr;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnLong;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnSByte;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnShort;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnString;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnUInt;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnUIntPtr;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnULong;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_ReturnUShort;
    extern LuaInvokeSite kSite_LuaInvokeMarshalProbe_SumInt;
    extern LuaInvokeSite kSite_TC_LuaTestHost_RunAll;

    void InitLuaInvokeSites();
    void RegisterGeneratedInternalCalls();
}
