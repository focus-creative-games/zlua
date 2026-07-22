#include "../lvm/LuaEnv.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/Marshaling.h"
#include "../marshal/ArrayMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/DelegateMarshal.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/OpaqueValueMarshal.h"
#include "../bridge/LuaInvokeHelper.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"
#include "../utils/LuaStackGuard.h"

#include "vm/Class.h"
#include "vm/InternalCalls.h"

// Point2D_086B8D836FA9D523F610DBBFA3EAC49D ZLua.Tests.Fixtures.Point2D
struct Point2D_086B8D836FA9D523F610DBBFA3EAC49D {
	int32_t __0; // X
	int32_t __1; // Y
};
// Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 UnityEngine.Vector2
struct Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 {
	float __0; // x
	float __1; // y
};
// Vector3_C8815E3C46FAA9E878AFFF2E09238741 UnityEngine.Vector3
struct Vector3_C8815E3C46FAA9E878AFFF2E09238741 {
	float __0; // x
	float __1; // y
	float __2; // z
};
// Vector4_C2796094C1DE915CAB7A0A45AB577CD3 UnityEngine.Vector4
struct Vector4_C2796094C1DE915CAB7A0A45AB577CD3 {
	float __0; // x
	float __1; // y
	float __2; // z
	float __3; // w
};
namespace zlua
{
namespace luainvoke
{
    
    // System.Byte[] ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoByteArrayBytes(System.Byte[])
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1(Il2CppArray* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "echo_byte_array_bytes", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoByteArrayBytes(System.Byte[])",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1 = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1.funcRef);
        ArrayMarshal::PushAsBytes(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = ArrayMarshal::PopFromBytes(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1->retMeta->typeKlass);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntDefault(System.Int32)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C(int32_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "echo_int_default", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntDefault(System.Int32)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C.funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidBytes(System.Int32)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1(int32_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "echo_int_invalid_bytes", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidBytes(System.Int32)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1.funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidUserData(System.Int32)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F(int32_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "echo_int_invalid_userdata", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidUserData(System.Int32)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F.funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.String ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoStringUserData(System.String)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80;
    
    static Il2CppString* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80(Il2CppString* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "echo_string_userdata", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoStringUserData(System.String)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80 = true;
        }
        Il2CppString* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80.funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = reinterpret_cast<Il2CppString*>(ObjectMarshal::Pop(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80->retMeta->typeKlass));
        return _retval;
    }
    // System.String ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ExpectStringUserData(System.String)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF;
    
    static Il2CppString* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF(Il2CppString* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "expect_string_userdata", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ExpectStringUserData(System.String)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF = true;
        }
        Il2CppString* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF.funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = StringMarshal::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReceiveStructOpaque(ZLua.Tests.Fixtures.Point2D)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB(Point2D_086B8D836FA9D523F610DBBFA3EAC49D __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "receive_struct_opaque", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReceiveStructOpaque(ZLua.Tests.Fixtures.Point2D)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
            OpaqueParameterScope opaqueScope;
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Byte[] ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReturnByteArrayBytes()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "return_byte_array_bytes", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReturnByteArrayBytes()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6 = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = ArrayMarshal::PopFromBytes(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6->retMeta->typeKlass);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::SumByteArrayBytes(System.Byte[])
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84(Il2CppArray* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal_as", "sum_byte_array_bytes", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::SumByteArrayBytes(System.Byte[])",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84.funcRef);
        ArrayMarshal::PushAsBytes(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Boolean ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoBool(System.Boolean)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3;
    
    static bool IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3(bool __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_bool", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoBool(System.Boolean)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3 = true;
        }
        bool _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3.funcRef);
        DefaultMarshaling<bool>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    // System.Byte[] ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByteArray(System.Byte[])
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B(Il2CppArray* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_byte_array", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByteArray(System.Byte[])",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B.funcRef);
        ArrayMarshal::PushAsArrayObject(L, __p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = ArrayMarshal::PopFromArrayObjectOrTable(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B->retMeta->typeKlass);
        return _retval;
    }
    // System.Byte ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByte(System.Byte)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7;
    
    static uint8_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7(uint8_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_byte", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByte(System.Byte)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7 = true;
        }
        uint8_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7.funcRef);
        DefaultMarshaling<uint8_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<uint8_t>::Pop(L, -1);
        return _retval;
    }
    // System.Char ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoChar(System.Char)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60;
    
    static uint16_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60(uint16_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_char", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoChar(System.Char)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60 = true;
        }
        uint16_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60.funcRef);
        DefaultMarshaling<uint16_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<uint16_t>::Pop(L, -1);
        return _retval;
    }
    // System.Double ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoDouble(System.Double)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19;
    
    static double IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19(double __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_double", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoDouble(System.Double)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19 = true;
        }
        double _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19.funcRef);
        DefaultMarshaling<double>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<double>::Pop(L, -1);
        return _retval;
    }
    // System.Single ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoFloat(System.Single)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB;
    
    static float IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB(float __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_float", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoFloat(System.Single)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB = true;
        }
        float _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB.funcRef);
        DefaultMarshaling<float>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<float>::Pop(L, -1);
        return _retval;
    }
    // System.Int32[] ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntArray(System.Int32[])
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858(Il2CppArray* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_int_array", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntArray(System.Int32[])",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858 = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858.funcRef);
        ArrayMarshal::PushAsArrayObject(L, __p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = ArrayMarshal::PopFromArrayObjectOrTable(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858->retMeta->typeKlass);
        return _retval;
    }
    // System.IntPtr ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntPtr(System.IntPtr)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9;
    
    static intptr_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9(intptr_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_intptr", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntPtr(System.IntPtr)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9 = true;
        }
        intptr_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9.funcRef);
        DefaultMarshaling<intptr_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<intptr_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoInt(System.Int32)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202(int32_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_int", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoInt(System.Int32)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202.funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int64 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoLong(System.Int64)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154;
    
    static int64_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154(int64_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_long", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoLong(System.Int64)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154 = true;
        }
        int64_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154.funcRef);
        DefaultMarshaling<int64_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int64_t>::Pop(L, -1);
        return _retval;
    }
    // System.SByte ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoSByte(System.SByte)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F;
    
    static int8_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F(int8_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_sbyte", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoSByte(System.SByte)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F = true;
        }
        int8_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F.funcRef);
        DefaultMarshaling<int8_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int8_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int16 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoShort(System.Int16)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D;
    
    static int16_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D(int16_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_short", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoShort(System.Int16)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D = true;
        }
        int16_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D.funcRef);
        DefaultMarshaling<int16_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int16_t>::Pop(L, -1);
        return _retval;
    }
    // System.String[] ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoStringArray(System.String[])
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8(Il2CppArray* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_string_array", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoStringArray(System.String[])",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8 = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8.funcRef);
        ArrayMarshal::PushAsArrayObject(L, __p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = ArrayMarshal::PopFromArrayObjectOrTable(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8->retMeta->typeKlass);
        return _retval;
    }
    // System.String ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoString(System.String)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD;
    
    static Il2CppString* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD(Il2CppString* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_string", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoString(System.String)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD = true;
        }
        Il2CppString* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD.funcRef);
        StringMarshal::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = StringMarshal::Pop(L, -1);
        return _retval;
    }
    // System.UIntPtr ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUIntPtr(System.UIntPtr)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835;
    
    static uintptr_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835(uintptr_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_uintptr", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUIntPtr(System.UIntPtr)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835 = true;
        }
        uintptr_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835.funcRef);
        DefaultMarshaling<uintptr_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<uintptr_t>::Pop(L, -1);
        return _retval;
    }
    // System.UInt32 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUInt(System.UInt32)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30;
    
    static uint32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30(uint32_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_uint", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUInt(System.UInt32)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30 = true;
        }
        uint32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30.funcRef);
        DefaultMarshaling<uint32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<uint32_t>::Pop(L, -1);
        return _retval;
    }
    // System.UInt64 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoULong(System.UInt64)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8;
    
    static uint64_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8(uint64_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_ulong", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoULong(System.UInt64)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8 = true;
        }
        uint64_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8.funcRef);
        DefaultMarshaling<uint64_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<uint64_t>::Pop(L, -1);
        return _retval;
    }
    // System.UInt16 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUShort(System.UInt16)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC;
    
    static uint16_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC(uint16_t __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "echo_ushort", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUShort(System.UInt16)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC = true;
        }
        uint16_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC.funcRef);
        DefaultMarshaling<uint16_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<uint16_t>::Pop(L, -1);
        return _retval;
    }
    // System.Void ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::Noop()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924;
    
    static void IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "noop", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::Noop()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924 = true;
        }
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924.funcRef);
        LuaUtil::PCall(L, 0, 0, errfunc);
    }
    // System.Boolean ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnBool()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B;
    
    static bool IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_bool", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnBool()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B = true;
        }
        bool _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    // System.Byte[] ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByteArray()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_byte_array", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByteArray()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = ArrayMarshal::PopFromArrayObjectOrTable(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA->retMeta->typeKlass);
        return _retval;
    }
    // System.Byte ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByte()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89;
    
    static uint8_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_byte", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByte()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89 = true;
        }
        uint8_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<uint8_t>::Pop(L, -1);
        return _retval;
    }
    // System.Char ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnChar()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D;
    
    static uint16_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_char", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnChar()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D = true;
        }
        uint16_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<uint16_t>::Pop(L, -1);
        return _retval;
    }
    // System.Double ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnDouble()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7;
    
    static double IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_double", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnDouble()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7 = true;
        }
        double _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<double>::Pop(L, -1);
        return _retval;
    }
    // ZLua.Tests.Fixtures.ColorKind ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnEnum()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_enum", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnEnum()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Single ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnFloat()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E;
    
    static float IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_float", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnFloat()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E = true;
        }
        float _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<float>::Pop(L, -1);
        return _retval;
    }
    // System.Int32[] ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntArray()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8;
    
    static Il2CppArray* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_int_array", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntArray()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8 = true;
        }
        Il2CppArray* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = ArrayMarshal::PopFromArrayObjectOrTable(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8->retMeta->typeKlass);
        return _retval;
    }
    // System.IntPtr ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntPtr()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000;
    
    static intptr_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_intptr", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntPtr()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000 = true;
        }
        intptr_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<intptr_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnInt()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_int", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnInt()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int64 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnLong()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA;
    
    static int64_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_long", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnLong()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA = true;
        }
        int64_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<int64_t>::Pop(L, -1);
        return _retval;
    }
    // System.SByte ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnSByte()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282;
    
    static int8_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_sbyte", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnSByte()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282 = true;
        }
        int8_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<int8_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int16 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnShort()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28;
    
    static int16_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_short", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnShort()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28 = true;
        }
        int16_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<int16_t>::Pop(L, -1);
        return _retval;
    }
    // System.String ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnString()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0;
    
    static Il2CppString* IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_string", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnString()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0 = true;
        }
        Il2CppString* _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = StringMarshal::Pop(L, -1);
        return _retval;
    }
    // System.UIntPtr ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUIntPtr()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680;
    
    static uintptr_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_uintptr", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUIntPtr()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680 = true;
        }
        uintptr_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<uintptr_t>::Pop(L, -1);
        return _retval;
    }
    // System.UInt32 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUInt()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E;
    
    static uint32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_uint", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUInt()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E = true;
        }
        uint32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<uint32_t>::Pop(L, -1);
        return _retval;
    }
    // System.UInt64 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnULong()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873;
    
    static uint64_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_ulong", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnULong()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873 = true;
        }
        uint64_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<uint64_t>::Pop(L, -1);
        return _retval;
    }
    // System.UInt16 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUShort()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466;
    
    static uint16_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "return_ushort", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUShort()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466 = true;
        }
        uint16_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        _retval = DefaultMarshaling<uint16_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumByteArray(System.Byte[])
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C(Il2CppArray* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "sum_byte_array", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumByteArray(System.Byte[])",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C.funcRef);
        ArrayMarshal::PushAsArrayObject(L, __p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumInt(System.Int32,System.Int32)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130;
    
    static int32_t IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130(int32_t __p0, int32_t __p1)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.marshal", "sum_int", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumInt(System.Int32,System.Int32)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130.funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        DefaultMarshaling<int32_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // UnityEngine.Vector3 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::AddVector3(UnityEngine.Vector3,UnityEngine.Vector3)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4;
    
    static Vector3_C8815E3C46FAA9E878AFFF2E09238741 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4(Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p0, Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p1)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "add_vector3", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::AddVector3(UnityEngine.Vector3,UnityEngine.Vector3)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4 = true;
        }
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4->paramsMeta[0]));
        StructMarshal::PushValue(L, &__p1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4->paramsMeta[1]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4->paramsMeta[1]));
        LuaUtil::PCall(L, 2, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // UnityEngine.Vector2 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector2(UnityEngine.Vector2)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A;
    
    static Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A(Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "echo_vector2", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector2(UnityEngine.Vector2)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A = true;
        }
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // UnityEngine.Vector3 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector3(UnityEngine.Vector3)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC;
    
    static Vector3_C8815E3C46FAA9E878AFFF2E09238741 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC(Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "echo_vector3", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector3(UnityEngine.Vector3)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC = true;
        }
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // UnityEngine.Vector4 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector4(UnityEngine.Vector4)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F;
    
    static Vector4_C2796094C1DE915CAB7A0A45AB577CD3 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F(Vector4_C2796094C1DE915CAB7A0A45AB577CD3 __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "echo_vector4", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector4(UnityEngine.Vector4)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F = true;
        }
        Vector4_C2796094C1DE915CAB7A0A45AB577CD3 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // UnityEngine.Vector2 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector2()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206;
    
    static Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "return_vector2", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector2()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206 = true;
        }
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // UnityEngine.Vector3 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector3()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C;
    
    static Vector3_C8815E3C46FAA9E878AFFF2E09238741 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "return_vector3", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector3()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C = true;
        }
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // UnityEngine.Vector4 ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector4()
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015;
    
    static Vector4_C2796094C1DE915CAB7A0A45AB577CD3 IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015()
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "return_vector4", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector4()",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015 = true;
        }
        Vector4_C2796094C1DE915CAB7A0A45AB577CD3 _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015.funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        StructMarshal::PopValue(L, -1, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015->retMeta->typeKlass, &_retval);
        return _retval;
    }
    // System.Single ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector2(UnityEngine.Vector2)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA;
    
    static float IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA(Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "sum_vector2", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector2(UnityEngine.Vector2)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA = true;
        }
        float _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<float>::Pop(L, -1);
        return _retval;
    }
    // System.Single ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector3(UnityEngine.Vector3)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372;
    
    static float IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372(Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "sum_vector3", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector3(UnityEngine.Vector3)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372 = true;
        }
        float _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<float>::Pop(L, -1);
        return _retval;
    }
    // System.Single ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector4(UnityEngine.Vector4)
    static bool kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA;
    
    static float IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA(Vector4_C2796094C1DE915CAB7A0A45AB577CD3 __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.unity_vector", "sum_vector4", "ZLua.Tests", "ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector4(UnityEngine.Vector4)",
                    kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA);
            kInitialized_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA = true;
        }
        float _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA.funcRef);
        StructMarshal::PushValue(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, kMethodMarshalCtx_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<float>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::BumpRefInt(System.Int32&)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7(void* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "bump_ref_int", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::BumpRefInt(System.Int32&)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::EchoOpaqueInt(System.Int32&)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143(void* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "echo_opaque_int", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::EchoOpaqueInt(System.Int32&)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardIntOpaqueManual(System.Int32&)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997(void* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "forward_int_opaque_manual", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardIntOpaqueManual(System.Int32&)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardIntOpaqueRaw(System.Int32&)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203(void* __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "forward_int_opaque_raw", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardIntOpaqueRaw(System.Int32&)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardPointOpaqueRaw(ZLua.Tests.Fixtures.Point2D)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44(Point2D_086B8D836FA9D523F610DBBFA3EAC49D __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "forward_point_opaque_raw", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardPointOpaqueRaw(ZLua.Tests.Fixtures.Point2D)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44 = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
            OpaqueParameterScope opaqueScope;
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::RoundtripOpaqueInt(System.Int32&,System.Int32)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA(void* __p0, int32_t __p1)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "roundtrip_opaque_int", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::RoundtripOpaqueInt(System.Int32&,System.Int32)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA->paramsMeta[0]->type);
        DefaultMarshaling<int32_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Int32 ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::SumPointOpaque(ZLua.Tests.Fixtures.Point2D)
    static bool kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F;
    static LuaInvokeSite kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F;
    
    static int32_t IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F(Point2D_086B8D836FA9D523F610DBBFA3EAC49D __p0)
    {
        if (!kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "test_luainvoke.opaque", "sum_point_opaque", "ZLua.Tests", "ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::SumPointOpaque(ZLua.Tests.Fixtures.Point2D)",
                    kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F);
            kInitialized_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F = true;
        }
        int32_t _retval{};
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
            OpaqueParameterScope opaqueScope;
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F.funcRef);
        OpaqueValueMarshal::Push(L, &__p0, kMethodMarshalCtx_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F->paramsMeta[0]->type);
        LuaUtil::PCall(L, 1, 1, errfunc);
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    // System.Void ZLua.Tests.TC_LuaTestHost::RunAll()
    static bool kInitialized_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F;
    static LuaInvokeSite kSite_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F;
    
    static void IC_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F()
    {
        if (!kInitialized_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "luatest.run_all", "run", "ZLua.Tests", "ZLua.Tests.TC_LuaTestHost::RunAll()",
                    kSite_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F, kMethodMarshalCtx_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F);
            kInitialized_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F = true;
        }
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F.funcRef);
        LuaUtil::PCall(L, 0, 0, errfunc);
    }
    // System.Void ZLua.Tests.TestBootstrap::Init()
    static bool kInitialized_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378;
    static LuaInvokeSite kSite_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378;
    static const MethodMarshalCtx* kMethodMarshalCtx_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378;
    
    static void IC_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378()
    {
        if (!kInitialized_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378)
        {
            LuaInvokeHelper::ResolveInvokeSiteAndMethod(LuaEnv::GetState(), "bootstrap", "init", "ZLua.Tests", "ZLua.Tests.TestBootstrap::Init()",
                    kSite_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378, kMethodMarshalCtx_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378);
            kInitialized_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378 = true;
        }
        lua_State* L = LuaEnv::GetState();
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, kSite_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378.funcRef);
        LuaUtil::PCall(L, 0, 0, errfunc);
    }
    void RegisterGeneratedInternalCalls()
    {
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoByteArrayBytes(System.Byte[])", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoByteArrayBytes_EBD9468F715DABB41D18958F9D452CF1);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntDefault(System.Int32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntDefault_1E8E4311227AC78B27072E083AEF883C);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidBytes(System.Int32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidBytes_286EBD57331BF69364AE86AFB03CF6C1);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoIntInvalidUserData(System.Int32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoIntInvalidUserData_9E56718AB3D5B7144A13F57EF1340A0F);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::EchoStringUserData(System.String)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_EchoStringUserData_B10F95B45940E881BA9BCA0AC155BD80);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ExpectStringUserData(System.String)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ExpectStringUserData_2BF9CDEF4DD0C60CD3488643B38A7DDF);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReceiveStructOpaque(ZLua.Tests.Fixtures.Point2D)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaque_928C63EE57BB3C5DF0A4ABB33148BABB);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::ReturnByteArrayBytes()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReturnByteArrayBytes_32CB3278E14D25AAC3CA1276390D1AC6);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalAsProbe::SumByteArrayBytes(System.Byte[])", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_SumByteArrayBytes_A67DFE7B8618A02D06801B8F18169E84);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoBool(System.Boolean)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoBool_037DAB4DF00FCF7A63A785540218AAA3);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByteArray(System.Byte[])", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByteArray_93C4A818E550FD67FD69B0C66182660B);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoByte(System.Byte)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoByte_0F5653C84343ADC383A006BACF4B34A7);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoChar(System.Char)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoChar_803063A68FDF20DA185E678A588DEE60);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoDouble(System.Double)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoDouble_2600FC4B39FEC42A13D24B69736EAB19);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoFloat(System.Single)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoFloat_D882E51C9D82455C86DBA70C1FA197FB);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntArray(System.Int32[])", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntArray_D2178B8E2357F638472EF0D5B34C1858);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoIntPtr(System.IntPtr)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoIntPtr_8EA762103EFFAF2A0D85746961DBE6B9);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoInt(System.Int32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoInt_E4921C16546C2F70EC1BBF52AABB2202);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoLong(System.Int64)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoLong_B6C39BC3C50E4CD84ABAF950F051E154);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoSByte(System.SByte)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoSByte_A429C053229C2F6B86BF642A5759C73F);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoShort(System.Int16)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoShort_401FE6AF749E527A98229ABFC5263D4D);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoStringArray(System.String[])", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoStringArray_D70D2556FFD513F36913F94E3ABECAD8);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoString(System.String)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoString_FC296D96ABC1E88F343598D15502EFFD);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUIntPtr(System.UIntPtr)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUIntPtr_B8DCBC96F2B0FC8B9845C25A9FCA4835);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUInt(System.UInt32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUInt_87FCD78C97AB3E0C35E2CC0F2C884C30);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoULong(System.UInt64)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoULong_9E112F085E4843A1B4399427A43051E8);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::EchoUShort(System.UInt16)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_EchoUShort_A7B125EFC0264BB7DB5D40E5B9193CBC);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::Noop()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_Noop_A8712BBA6E85EABA09EDFF291A68E924);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnBool()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnBool_A142B4CE42361D857BD1CA26C427B88B);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByteArray()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByteArray_1F5EE555236C30A28F694B06D953A6DA);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnByte()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnByte_9A60B36660A71E32CDC816EE1C70FF89);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnChar()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnChar_5A0F817646E2819913F5D81EE09F673D);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnDouble()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnDouble_5AF34ED250B901E0822AF59CD64962D7);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnEnum()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnEnum_CEE8FF00E29A555AA4352BF8CCD9ABDF);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnFloat()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnFloat_5CE75E288320514ACB6252CA3C1B808E);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntArray()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntArray_F29BEA65DD3C71C8393E5598F8513CB8);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnIntPtr()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnIntPtr_373EAD5DE04050BD465F7DEA58520000);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnInt()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnInt_68DF4139912B291A1CC20430DEC0F7A8);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnLong()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnLong_2D324D3C01B76E77BE8925BC82B1C1BA);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnSByte()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnSByte_611092F5E7113A05D90BE9502463A282);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnShort()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnShort_B105E8A3ABB0ACBC2FA9604580F50B28);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnString()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnString_C146EA099C1076C676988D6DB8E992E0);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUIntPtr()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUIntPtr_F18E23F4AF03BCEB9D8E37B253903680);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUInt()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUInt_10EBE1A30B79D3C20ED7F06F35E9B50E);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnULong()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnULong_2FA51A1B8D3F6C454652AFC3C1419873);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::ReturnUShort()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_ReturnUShort_2424B3B38FE7696E427E1A1274EDD466);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumByteArray(System.Byte[])", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumByteArray_A43D56361D82F3BAA009824FA552401C);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeMarshalProbe::SumInt(System.Int32,System.Int32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeMarshalProbe_SumInt_D177B6027F34ACDF26A751D654FD6130);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::AddVector3(UnityEngine.Vector3,UnityEngine.Vector3)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_AddVector3_7C992E74DAD0CF852398E9CC922367D4);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector2(UnityEngine.Vector2)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector2_78FC0815265A56A8AB456C46D8F4187A);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector3(UnityEngine.Vector3)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector3_10A5D622EDFF90F33CBACF99818F72BC);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::EchoVector4(UnityEngine.Vector4)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_EchoVector4_32F85D158CC4A069599177D80C69197F);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector2()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector2_E5DC8A6FB92BD9A73B388302C0C44206);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector3()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector3_81A210DF5DA7A76E22FB75401C0D0A2C);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::ReturnVector4()", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_ReturnVector4_2119AFF3CFDEFAB56CC4724E28ED4015);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector2(UnityEngine.Vector2)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector2_4FCA41B286A8247C88E0794E53B98DAA);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector3(UnityEngine.Vector3)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector3_E4B57AAEE802A6EEFE570FDFDD988372);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.LuaInvokeUnityVectorProbe::SumVector4(UnityEngine.Vector4)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_LuaInvokeUnityVectorProbe_SumVector4_E63B044B764A03E3B693B4B434D930AA);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::BumpRefInt(System.Int32&)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefInt_97694458FC67DC4F4D5B56FE80C564A7);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::EchoOpaqueInt(System.Int32&)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_EchoOpaqueInt_D97A7E246426E724B66DDE9CF9D11143);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardIntOpaqueManual(System.Int32&)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueManual_67E15CBFB53863F323B930A8E5D61997);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardIntOpaqueRaw(System.Int32&)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardIntOpaqueRaw_70093D645D56A2249DE9E1811D0B2203);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::ForwardPointOpaqueRaw(ZLua.Tests.Fixtures.Point2D)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_ForwardPointOpaqueRaw_375AFDB334083F152767F38654630E44);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::RoundtripOpaqueInt(System.Int32&,System.Int32)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueInt_ED2C47E37DBFA1AC7B66E63E909112BA);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.Fixtures.OpaqueValueMarshalProbe::SumPointOpaque(ZLua.Tests.Fixtures.Point2D)", (Il2CppMethodPointer)IC_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_SumPointOpaque_05124F7DA9BE5B9DB6C1987758D7C49F);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.TC_LuaTestHost::RunAll()", (Il2CppMethodPointer)IC_ZLua_Tests_TC_LuaTestHost_RunAll_083D01C6D08AD103B9A337CE185F477F);
        il2cpp::vm::InternalCalls::Add("ZLua.Tests.TestBootstrap::Init()", (Il2CppMethodPointer)IC_ZLua_Tests_TestBootstrap_Init_03524685D5B32BED71582858A7807378);
    }
} // namespace luainvoke
} // namespace zlua
