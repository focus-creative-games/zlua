#include "DelegateBridgeStub.h"

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
#include "../mt/MetaBinding.h"

namespace zlua
{
namespace delegatebridge
{
    // NativeArray_601_1B35A214635032039737367046A578BF Unity.Collections.NativeArray`1<System.Byte>
    struct NativeArray_601_1B35A214635032039737367046A578BF {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // DebugScreenCapture_D801C2E81C22E70B43AF647AB6D62D90 Unity.Profiling.DebugScreenCapture
    struct DebugScreenCapture_D801C2E81C22E70B43AF647AB6D62D90 {
    	NativeArray_601_1B35A214635032039737367046A578BF __0; // <RawImageDataReference>k__BackingField
    	int32_t __1; // <ImageFormat>k__BackingField
    	int32_t __2; // <Width>k__BackingField
    	int32_t __3; // <Height>k__BackingField
    };
    // InputDevice_48A4DCB3CD89C07D83DC681C7063C920 UnityEngine.XR.InputDevice
    struct InputDevice_48A4DCB3CD89C07D83DC681C7063C920 {
    	uint64_t __0; // m_DeviceId
    	bool __1; // m_Initialized
    };
    // Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 UnityEngine.Vector2
    struct Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 {
    	float __0; // x
    	float __1; // y
    };
    // KeyValuePair_602_73B408C6BD697DD662918B33E1DC2A02 System.Collections.Generic.KeyValuePair`2<System.Int32,UnityEngine.Vector2>
    struct KeyValuePair_602_73B408C6BD697DD662918B33E1DC2A02 {
    	int32_t __0; // key
    	Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 __1; // value
    };
    // KeyValuePair_602_9237FDD8C09D4CA782AA47606CD393A6 System.Collections.Generic.KeyValuePair`2<System.String,System.String>
    struct KeyValuePair_602_9237FDD8C09D4CA782AA47606CD393A6 {
    	Il2CppObject* __0; // key
    	Il2CppObject* __1; // value
    };
    // MeshId_84B6E7CB922FD04C888FB8214E7767BC UnityEngine.XR.MeshId
    struct MeshId_84B6E7CB922FD04C888FB8214E7767BC {
    	uint64_t __0; // m_SubId1
    	uint64_t __1; // m_SubId2
    };
    // Vector3_C8815E3C46FAA9E878AFFF2E09238741 UnityEngine.Vector3
    struct Vector3_C8815E3C46FAA9E878AFFF2E09238741 {
    	float __0; // x
    	float __1; // y
    	float __2; // z
    };
    // Quaternion_06C43E4813DAE969B7B6C1DE1AB46BF7 UnityEngine.Quaternion
    struct Quaternion_06C43E4813DAE969B7B6C1DE1AB46BF7 {
    	float __0; // x
    	float __1; // y
    	float __2; // z
    	float __3; // w
    };
    // MeshGenerationResult_CD98C63E8CC2D2E24E030DDD9B94E579 UnityEngine.XR.MeshGenerationResult
    struct MeshGenerationResult_CD98C63E8CC2D2E24E030DDD9B94E579 {
    	MeshId_84B6E7CB922FD04C888FB8214E7767BC __0; // <MeshId>k__BackingField
    	Il2CppObject* __1; // <Mesh>k__BackingField
    	Il2CppObject* __2; // <MeshCollider>k__BackingField
    	int32_t __3; // <Status>k__BackingField
    	int32_t __4; // <Attributes>k__BackingField
    	uint64_t __5; // <Timestamp>k__BackingField
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __6; // <Position>k__BackingField
    	Quaternion_06C43E4813DAE969B7B6C1DE1AB46BF7 __7; // <Rotation>k__BackingField
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __8; // <Scale>k__BackingField
    };
    // NativeArray_601_0184FEB69BECD9A48DCE3C28B22B8DB5 Unity.Collections.NativeArray`1<UnityEngine.Vector3Int>
    struct NativeArray_601_0184FEB69BECD9A48DCE3C28B22B8DB5 {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // NativeArray_601_D0529AAB3CC1DB21D7B7C43A7EF16D47 Unity.Collections.NativeArray`1<UnityEngine.ModifiableContactPair>
    struct NativeArray_601_D0529AAB3CC1DB21D7B7C43A7EF16D47 {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // PhysicsScene_7789191B084D38B36003CA2FFB99B451 UnityEngine.PhysicsScene
    struct PhysicsScene_7789191B084D38B36003CA2FFB99B451 {
    	int32_t __0; // m_Handle
    };
    // Scene_E7331A80F14DA8FDE306F5639BD2AD98 UnityEngine.SceneManagement.Scene
    struct Scene_E7331A80F14DA8FDE306F5639BD2AD98 {
    	int32_t __0; // m_Handle
    };
    // ByReference_601_9CF0EC17524460FD392C29AE69B20BE6 System.ByReference`1<System.Char>
    struct ByReference_601_9CF0EC17524460FD392C29AE69B20BE6 {
    	intptr_t __0; // _value
    };
    // Span_601_B60174942194C75AB65CF0728AF98DA1 System.Span`1<System.Char>
    struct Span_601_B60174942194C75AB65CF0728AF98DA1 {
    	ByReference_601_9CF0EC17524460FD392C29AE69B20BE6 __0; // _pointer
    	int32_t __1; // _length
    };
    // StreamingContext_9EA1561C462D8798D782A597A7649302 System.Runtime.Serialization.StreamingContext
    struct StreamingContext_9EA1561C462D8798D782A597A7649302 {
    	Il2CppObject* __0; // m_additionalContext
    	int32_t __1; // m_state
    };
    // NativeArray_601_51729D6BB72EB052B0766F438E799D4B Unity.Collections.NativeArray`1<System.Int32>
    struct NativeArray_601_51729D6BB72EB052B0766F438E799D4B {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // NativeArray_601_C59CA0D712D7F7C5AE1A3AD9766101D5 Unity.Collections.NativeArray`1<UnityEngine.Matrix4x4>
    struct NativeArray_601_C59CA0D712D7F7C5AE1A3AD9766101D5 {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // NativeArray_601_395436D45740A04D74D252A7995379DC Unity.Collections.NativeArray`1<UnityEngine.Vector3>
    struct NativeArray_601_395436D45740A04D74D252A7995379DC {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // NativeArray_601_65F8B75C1C05CEED76892D02C361D0BC Unity.Collections.NativeArray`1<UnityEngine.Quaternion>
    struct NativeArray_601_65F8B75C1C05CEED76892D02C361D0BC {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // TransformDispatchData_DB423FDFE6DDD2E3125F1494BAFA4255 UnityEngine.TransformDispatchData
    struct TransformDispatchData_DB423FDFE6DDD2E3125F1494BAFA4255 {
    	NativeArray_601_51729D6BB72EB052B0766F438E799D4B __0; // transformedID
    	NativeArray_601_51729D6BB72EB052B0766F438E799D4B __1; // parentID
    	NativeArray_601_C59CA0D712D7F7C5AE1A3AD9766101D5 __2; // localToWorldMatrices
    	NativeArray_601_395436D45740A04D74D252A7995379DC __3; // positions
    	NativeArray_601_65F8B75C1C05CEED76892D02C361D0BC __4; // rotations
    	NativeArray_601_395436D45740A04D74D252A7995379DC __5; // scales
    };
    // TypeDispatchData_9A31C2DC5F05F365A3887692758FE3DF UnityEngine.TypeDispatchData
    struct TypeDispatchData_9A31C2DC5F05F365A3887692758FE3DF {
    	Il2CppObject* __0; // changed
    	NativeArray_601_51729D6BB72EB052B0766F438E799D4B __1; // changedID
    	NativeArray_601_51729D6BB72EB052B0766F438E799D4B __2; // destroyedID
    };
    // ValueTuple_603_1C8AB44998962ACBB01CF329D415EB4C System.ValueTuple`3<System.Byte[],System.Int32,System.Int32>
    struct ValueTuple_603_1C8AB44998962ACBB01CF329D415EB4C {
    	Il2CppObject* __0; // Item1
    	int32_t __1; // Item2
    	int32_t __2; // Item3
    };
    // VFXOutputEventArgs_A3C7FD2AFF134B39596176EF77949446 UnityEngine.VFX.VFXOutputEventArgs
    struct VFXOutputEventArgs_A3C7FD2AFF134B39596176EF77949446 {
    	int32_t __0; // <nameId>k__BackingField
    	Il2CppObject* __1; // <eventAttribute>k__BackingField
    };
    // XRNodeState_F6DEE909529BF35FD47D4041534B2A53 UnityEngine.XR.XRNodeState
    struct XRNodeState_F6DEE909529BF35FD47D4041534B2A53 {
    	int32_t __0; // m_Type
    	int32_t __1; // m_AvailableFields
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __2; // m_Position
    	Quaternion_06C43E4813DAE969B7B6C1DE1AB46BF7 __3; // m_Rotation
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __4; // m_Velocity
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __5; // m_AngularVelocity
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __6; // m_Acceleration
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __7; // m_AngularAcceleration
    	int32_t __8; // m_Tracked
    	uint64_t __9; // m_UniqueID
    };
    static void InvokeImpl_System_Action_Invoke_FE05452379D12DAF1D88F5EA570E46C1(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_A39F452680A1B20BC60054BA102E0069(Il2CppObject* target, bool __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<bool>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_24C629DDF46CAFAEE7C2BD96E856EDE1(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_EC0CA2AB4907826B3CFB12E5D3146419(Il2CppObject* target, intptr_t __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<intptr_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_41AECDF51E0628BC523C0BDA09BE7ED6(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_01C3B4E91E7BB8D2AE1C2BDEDC92F34A(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StringMarshal::Push(L, __p0);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_98BCD4B5FE4794F54659567EA6CA69FD(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_DF2502038B158F7F1E02921CE830E0C5(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_21B1183073D1D88BE363D7A4693A5F13(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_FAE35AC2BBB990785B5FDA1BC3BC930A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_064555E25A2D5A3C2C6A4CA5217E9DCE(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_F67912BC361F8316BBB0C3057B44A207(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_B639C805DE04B9BE1BA03C6B092DC327(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_664DF8A1C001B12121AE3597F333BA19(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_D4CC203295D7E726DA4F9DABD3631060(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_65DB44D8166DC7F146B450887F064A0C(Il2CppObject* target, TransformDispatchData_DB423FDFE6DDD2E3125F1494BAFA4255 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_DE2F96FFD1B2C72B8D459E162B5CF4DD(Il2CppObject* target, TypeDispatchData_9A31C2DC5F05F365A3887692758FE3DF __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_35B9D8C50646D294D7DBC6F35B5E3F19(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_F61EBB59F65CD90B82969A9B33374E2B(Il2CppObject* target, VFXOutputEventArgs_A3C7FD2AFF134B39596176EF77949446 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_9F21937674EEF54E1104B39CE5F2ED3C(Il2CppObject* target, InputDevice_48A4DCB3CD89C07D83DC681C7063C920 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_7B77326C059E531A29259A53604148E7(Il2CppObject* target, MeshGenerationResult_CD98C63E8CC2D2E24E030DDD9B94E579 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_CAE58F5D4876726570C2457C2755D331(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_7A5105A2DEB7AB86C313BBD797DA9B65(Il2CppObject* target, XRNodeState_F6DEE909529BF35FD47D4041534B2A53 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_02DDACFD79937AA4A9FCB1F4C63FCC6C(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_75DB1211162494160E4D21CFCC571CD3(Il2CppObject* target, int32_t __p0, int32_t __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        DefaultMarshaling<int32_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_59B3143529826F3F27315CD6CCCEE9DD(Il2CppObject* target, int32_t __p0, Il2CppString* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        StringMarshal::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_965A6EF0D0686130905C53D1F68A25CC(Il2CppObject* target, Il2CppString* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StringMarshal::Push(L, __p0);
        DelegateMarshal::Push(L, reinterpret_cast<Il2CppDelegate*>(__p1), MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[1]));
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_5A9CA2189F1B9B549162F2A75A5B743F(Il2CppObject* target, Il2CppString* __p0, bool __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StringMarshal::Push(L, __p0);
        DefaultMarshaling<bool>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_61BAF5D906CAE0920A497716E1A0873B(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_EF280BF91C750277597B67406BE9A4C4(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_65245978CE7BBDA1BE40D205152EBB38(Il2CppObject* target, int32_t __p0, int32_t __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        DefaultMarshaling<int32_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_E09F3BDAD5FBF2E47531DBFDF2C5E6C1(Il2CppObject* target, PhysicsScene_7789191B084D38B36003CA2FFB99B451 __p0, NativeArray_601_D0529AAB3CC1DB21D7B7C43A7EF16D47 __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        StructMarshal::PushValue(L, &__p1, ctx->paramsMeta[1]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[1]));
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_F1A752D897FFE2047E986D383244CF65(Il2CppObject* target, Il2CppObject* __p0, int32_t __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        DefaultMarshaling<int32_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_90B5FFACA3DCCC58197939127C1BFCD4(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_04DC3443E818AE8304D85C30904C4693(Il2CppObject* target, Il2CppObject* __p0, NativeArray_601_0184FEB69BECD9A48DCE3C28B22B8DB5 __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        StructMarshal::PushValue(L, &__p1, ctx->paramsMeta[1]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[1]));
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_4C9086B0516ECC53926ECF01D04F9F6A(Il2CppObject* target, Il2CppObject* __p0, Il2CppArray* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ArrayMarshal::PushAsArrayObject(L, __p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_603_Invoke_DC030962EAB57F8748FFC2C45646B4F6(Il2CppObject* target, bool __p0, bool __p1, int32_t __p2, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<bool>::Push(L, __p0);
        DefaultMarshaling<bool>::Push(L, __p1);
        DefaultMarshaling<int32_t>::Push(L, __p2);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_System_Action_603_Invoke_D52448CCB9CCFBB5C3D3B91010391E01(Il2CppObject* target, Il2CppString* __p0, bool __p1, DebugScreenCapture_D801C2E81C22E70B43AF647AB6D62D90 __p2, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StringMarshal::Push(L, __p0);
        DefaultMarshaling<bool>::Push(L, __p1);
        StructMarshal::PushValue(L, &__p2, ctx->paramsMeta[2]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[2]));
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_System_Action_606_Invoke_CE304293C0004693C98A2C0C47E06784(Il2CppObject* target, Il2CppArray* __p0, intptr_t __p1, intptr_t __p2, int32_t __p3, int32_t __p4, Il2CppObject* __p5, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ArrayMarshal::PushAsArrayObject(L, __p0, ctx->paramsMeta[0]);
        DefaultMarshaling<intptr_t>::Push(L, __p1);
        DefaultMarshaling<intptr_t>::Push(L, __p2);
        DefaultMarshaling<int32_t>::Push(L, __p3);
        DefaultMarshaling<int32_t>::Push(L, __p4);
        DelegateMarshal::Push(L, reinterpret_cast<Il2CppDelegate*>(__p5), MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[5]));
        LuaUtil::PCall(L, 6, 0, errfunc);
    }
    static void InvokeImpl_System_Action_608_Invoke_068298F64FDBCFBE1B797266D472158A(Il2CppObject* target, intptr_t __p0, intptr_t __p1, intptr_t __p2, intptr_t __p3, intptr_t __p4, intptr_t __p5, int32_t __p6, Il2CppObject* __p7, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<intptr_t>::Push(L, __p0);
        DefaultMarshaling<intptr_t>::Push(L, __p1);
        DefaultMarshaling<intptr_t>::Push(L, __p2);
        DefaultMarshaling<intptr_t>::Push(L, __p3);
        DefaultMarshaling<intptr_t>::Push(L, __p4);
        DefaultMarshaling<intptr_t>::Push(L, __p5);
        DefaultMarshaling<int32_t>::Push(L, __p6);
        DelegateMarshal::Push(L, reinterpret_cast<Il2CppDelegate*>(__p7), MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[7]));
        LuaUtil::PCall(L, 8, 0, errfunc);
    }
    static void InvokeImpl_System_AssemblyLoadEventHandler_Invoke_CC41FBF96F7EC3415414DB537C69E874(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_AsyncCallback_Invoke_843A8AB41287023381A0F0929381A390(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Buffers_SpanAction_602_Invoke_02E6B8C8678F6D41ECB756E276A547D3(Il2CppObject* target, Span_601_B60174942194C75AB65CF0728AF98DA1 __p0, ValueTuple_603_1C8AB44998962ACBB01CF329D415EB4C __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        StructMarshal::PushValue(L, &__p1, ctx->paramsMeta[1]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[1]));
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static int32_t InvokeImpl_System_Comparison_601_Invoke_93FD44DD6A387F8EC6A76662D1B8AABA(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval;
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    static int32_t InvokeImpl_System_Comparison_601_Invoke_702EFBA29914C6663F211A284A4BD4FF(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval;
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    static int32_t InvokeImpl_System_Comparison_601_Invoke_EAD6050AF192A6946ED5269EED2D5BDF(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval;
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    static void InvokeImpl_System_ConsoleCancelEventHandler_Invoke_C53BC02A5B1D2EF801D4CDE444794C5A(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_EventHandler_Invoke_5D4BFE80FB11C3B5C8380F19CEEF8917(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_EventHandler_601_Invoke_0440C66B8C7965B59EB3720CB86F10DE(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_EventHandler_601_Invoke_C198F3ECC8382CF718B501AEF39591E9(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static bool InvokeImpl_System_Func_601_Invoke_7A3A9394E8B685A6A9BA2D05C7ECCC6E(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_AEA92837B43BDBFDB5B7AC92A7074CAA(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static KeyValuePair_602_73B408C6BD697DD662918B33E1DC2A02 InvokeImpl_System_Func_601_Invoke_3C4796851B6337FD4C5A04C11B434A07(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        KeyValuePair_602_73B408C6BD697DD662918B33E1DC2A02 _retval;
        StructMarshal::PopValue(L, -1, ctx->retMeta->typeKlass, &_retval);
        return _retval;
    }
    static Il2CppString* InvokeImpl_System_Func_601_Invoke_52884981E83073CE4CA2626128392C4C(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppString* _retval;
        _retval = StringMarshal::Pop(L, -1);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_BF119905D0E9126126C9F177271CCDE7(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_4650C80C8345EB36B42C5952D7E24ABE(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_322FC716ED9140502D4DB7A31A815477(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_7E5C998D2A63FC7ABF9D1425CC6E355C(Il2CppObject* target, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static Il2CppString* InvokeImpl_System_Func_602_Invoke_51612775A3FD238001795AE7F8774CEF(Il2CppObject* target, KeyValuePair_602_9237FDD8C09D4CA782AA47606CD393A6 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppString* _retval;
        _retval = StringMarshal::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_3DB697229945861A63E7646435551DAF(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_0CE2F8006A37402C00913A791941C2D8(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval;
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_C110A6E6DA39CEA93B02EDD35F30097A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_7E25632ABFB659591C9C23D4561D7980(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval;
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_B7F9C929D2D760F42603214EC9698F6B(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_0E9900F6C7CB2561D49C09C3AE7DE83E(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_E147B2747509D93ACAE5413079059217(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval;
        _retval = DefaultMarshaling<int32_t>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_51CEB3E46CAE6B3DF8A69AD83F729605(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_602_Invoke_E0C21A67BCFC4A75E3A1F30BCAA86CE3(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval;
        _retval = ArrayMarshal::PopFromArrayObjectOrTable(L, -1, ctx->retMeta->typeKlass);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_A30B508EC9E91D00EC90262FFC23435D(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StringMarshal::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_FB46503A990AD6F76C9A5BA91E43FDE4(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_8DE427C01B2A998665357D4AA354A544(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_441D5010B7E8DFDDCA783A8C59F48C33(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_F8D59E2D4A6B994D3776790B8428FC8A(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Func_603_Invoke_DBA73F3DF7E04ADA6EF414EAB3D99AD6(Il2CppObject* target, int32_t __p0, intptr_t __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        DefaultMarshaling<int32_t>::Push(L, __p0);
        DefaultMarshaling<intptr_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_603_Invoke_0AA728C1802D98A07EB17AC99F8EA9E2(Il2CppObject* target, Il2CppObject* __p0, Il2CppArray* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ArrayMarshal::PushAsArrayObject(L, __p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_604_Invoke_98A41561C238028C04C1ECC7C31D4236(Il2CppObject* target, Il2CppObject* __p0, Il2CppString* __p1, bool __p2, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        StringMarshal::Push(L, __p1);
        DefaultMarshaling<bool>::Push(L, __p2);
        LuaUtil::PCall(L, 3, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static bool InvokeImpl_System_Predicate_601_Invoke_3C3D7699BC9A85032411CAD8EFBAB7C3(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Predicate_601_Invoke_F0ADF08342507F130C6D901BCF1C619A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Predicate_601_Invoke_78F859C3DBC2E777ABA61B6A194FE916(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Reflection_MemberFilter_Invoke_95ED7EA54716BCDB3AF751AF3B389266(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static bool InvokeImpl_System_Reflection_TypeFilter_Invoke_F9BFBD61A8A5464849C311B7EC71A29A(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval;
        _retval = DefaultMarshaling<bool>::Pop(L, -1);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_ResolveEventHandler_Invoke_63D09AAB8B6FC6A81F37CC26A468C659(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Runtime_Remoting_Messaging_HeaderHandler_Invoke_BA219244F7112FD2556020ECAA9AD21A(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ArrayMarshal::PushAsArrayObject(L, __p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval;
        _retval = reinterpret_cast<Il2CppObject*>(ObjectMarshal::Pop(L, -1, ctx->retMeta->typeKlass));
        return _retval;
    }
    static void InvokeImpl_System_Runtime_Serialization_SerializationEventHandler_Invoke_64EA7307F018701E361E37716F8D55D7(Il2CppObject* target, StreamingContext_9EA1561C462D8798D782A597A7649302 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Threading_WaitOrTimerCallback_Invoke_4F7B35AC9F6CCBABAB79EECD22A326E9(Il2CppObject* target, Il2CppObject* __p0, bool __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        DefaultMarshaling<bool>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_UnhandledExceptionEventHandler_Invoke_B09F08BCB4B1BC377C8A356FB27BDC13(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p1), ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_C64FC774213E14E1AC95852F0A5DEF8E(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        ObjectMarshal::Push(L, reinterpret_cast<Il2CppObject*>(__p0), ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_9A02FA3FB1EF2ECE1B79DDCC8A04C4AD(Il2CppObject* target, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p0, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_47765518F738B82B22B2C889C2B5F732(Il2CppObject* target, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p0, int32_t __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        DefaultMarshaling<int32_t>::Push(L, __p1);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_DFAEF22D84FB1A5C41A9C4B75A41FD82(Il2CppObject* target, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p0, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p1, const MethodInfo* method)
    {
        static const MethodMarshalCtx* ctx = nullptr;
        lua_State* L = LuaEnv::GetState();
        if (ctx == nullptr)
        {
            ctx = MetaBinding::CreateMethodMarshalCtx(L, method, false);
        }
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        StructMarshal::PushValue(L, &__p0, ctx->paramsMeta[0]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[0]));
        StructMarshal::PushValue(L, &__p1, ctx->paramsMeta[1]->typeKlass, MarshalMeta::EnsureByValMetatableRef(L, ctx->paramsMeta[1]));
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    const DelegateBridgeEntry g_delegateBridges[] = {
        { "System.Void()", (Il2CppMethodPointer)InvokeImpl_System_Action_Invoke_FE05452379D12DAF1D88F5EA570E46C1 },
        { "System.Void(System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_A39F452680A1B20BC60054BA102E0069 },
        { "System.Void(System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_24C629DDF46CAFAEE7C2BD96E856EDE1 },
        { "System.Void(System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_EC0CA2AB4907826B3CFB12E5D3146419 },
        { "System.Void(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_41AECDF51E0628BC523C0BDA09BE7ED6 },
        { "System.Void(System.String)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_01C3B4E91E7BB8D2AE1C2BDEDC92F34A },
        { "System.Void(System.Text.StringBuilder)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_98BCD4B5FE4794F54659567EA6CA69FD },
        { "System.Void(System.Threading.Tasks.Task)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_DF2502038B158F7F1E02921CE830E0C5 },
        { "System.Void(Unity.Profiling.Memory.MemorySnapshotMetadata)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_21B1183073D1D88BE363D7A4693A5F13 },
        { "System.Void(UnityEngine.AsyncOperation)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_FAE35AC2BBB990785B5FDA1BC3BC930A },
        { "System.Void(UnityEngine.Camera)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_064555E25A2D5A3C2C6A4CA5217E9DCE },
        { "System.Void(UnityEngine.CustomRenderTexture)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_F67912BC361F8316BBB0C3057B44A207 },
        { "System.Void(UnityEngine.Font)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_B639C805DE04B9BE1BA03C6B092DC327 },
        { "System.Void(UnityEngine.Playables.PlayableDirector)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_664DF8A1C001B12121AE3597F333BA19 },
        { "System.Void(UnityEngine.Texture)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_D4CC203295D7E726DA4F9DABD3631060 },
        { "System.Void(UnityEngine.TransformDispatchData)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_65DB44D8166DC7F146B450887F064A0C },
        { "System.Void(UnityEngine.TypeDispatchData)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_DE2F96FFD1B2C72B8D459E162B5CF4DD },
        { "System.Void(UnityEngine.U2D.SpriteAtlas)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_35B9D8C50646D294D7DBC6F35B5E3F19 },
        { "System.Void(UnityEngine.VFX.VFXOutputEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_F61EBB59F65CD90B82969A9B33374E2B },
        { "System.Void(UnityEngine.XR.InputDevice)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_9F21937674EEF54E1104B39CE5F2ED3C },
        { "System.Void(UnityEngine.XR.MeshGenerationResult)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_7B77326C059E531A29259A53604148E7 },
        { "System.Void(UnityEngine.XR.XRInputSubsystem)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_CAE58F5D4876726570C2457C2755D331 },
        { "System.Void(UnityEngine.XR.XRNodeState)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_7A5105A2DEB7AB86C313BBD797DA9B65 },
        { "System.Void(UnityEngineInternal.Input.NativeInputUpdateType)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_02DDACFD79937AA4A9FCB1F4C63FCC6C },
        { "System.Void(System.Int32,System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_75DB1211162494160E4D21CFCC571CD3 },
        { "System.Void(System.Int32,System.String)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_59B3143529826F3F27315CD6CCCEE9DD },
        { "System.Void(System.String,System.Action`1<UnityEngine.U2D.SpriteAtlas>)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_965A6EF0D0686130905C53D1F68A25CC },
        { "System.Void(System.String,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_5A9CA2189F1B9B549162F2A75A5B743F },
        { "System.Void(System.Threading.Tasks.Task,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_61BAF5D906CAE0920A497716E1A0873B },
        { "System.Void(System.Type,Unity.Properties.IPropertyBag)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_EF280BF91C750277597B67406BE9A4C4 },
        { "System.Void(UnityEngine.EventType,UnityEngine.KeyCode)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_65245978CE7BBDA1BE40D205152EBB38 },
        { "System.Void(UnityEngine.PhysicsScene,Unity.Collections.NativeArray`1<UnityEngine.ModifiableContactPair>)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_E09F3BDAD5FBF2E47531DBFDF2C5E6C1 },
        { "System.Void(UnityEngine.ReflectionProbe,UnityEngine.ReflectionProbe+ReflectionProbeEvent)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_F1A752D897FFE2047E986D383244CF65 },
        { "System.Void(UnityEngine.Rendering.RenderPipelineAsset,UnityEngine.Rendering.RenderPipelineAsset)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_90B5FFACA3DCCC58197939127C1BFCD4 },
        { "System.Void(UnityEngine.Tilemaps.Tilemap,Unity.Collections.NativeArray`1<UnityEngine.Vector3Int>)", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_04DC3443E818AE8304D85C30904C4693 },
        { "System.Void(UnityEngine.Tilemaps.Tilemap,UnityEngine.Tilemaps.Tilemap+SyncTile[])", (Il2CppMethodPointer)InvokeImpl_System_Action_602_Invoke_4C9086B0516ECC53926ECF01D04F9F6A },
        { "System.Void(System.Boolean,System.Boolean,System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Action_603_Invoke_DC030962EAB57F8748FFC2C45646B4F6 },
        { "System.Void(System.String,System.Boolean,Unity.Profiling.DebugScreenCapture)", (Il2CppMethodPointer)InvokeImpl_System_Action_603_Invoke_D52448CCB9CCFBB5C3D3B91010391E01 },
        { "System.Void(UnityEngine.Object[],System.IntPtr,System.IntPtr,System.Int32,System.Int32,System.Action`1<UnityEngine.TypeDispatchData>)", (Il2CppMethodPointer)InvokeImpl_System_Action_606_Invoke_CE304293C0004693C98A2C0C47E06784 },
        { "System.Void(System.IntPtr,System.IntPtr,System.IntPtr,System.IntPtr,System.IntPtr,System.IntPtr,System.Int32,System.Action`1<UnityEngine.TransformDispatchData>)", (Il2CppMethodPointer)InvokeImpl_System_Action_608_Invoke_068298F64FDBCFBE1B797266D472158A },
        { "System.Void(System.Object,System.AssemblyLoadEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_AssemblyLoadEventHandler_Invoke_CC41FBF96F7EC3415414DB537C69E874 },
        { "System.Void(System.IAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_AsyncCallback_Invoke_843A8AB41287023381A0F0929381A390 },
        { "System.Void(System.Span`1<System.Char>,System.ValueTuple`3<System.Byte[],System.Int32,System.Int32>)", (Il2CppMethodPointer)InvokeImpl_System_Buffers_SpanAction_602_Invoke_02E6B8C8678F6D41ECB756E276A547D3 },
        { "System.Int32(Mono.Globalization.Unicode.Level2Map,Mono.Globalization.Unicode.Level2Map)", (Il2CppMethodPointer)InvokeImpl_System_Comparison_601_Invoke_93FD44DD6A387F8EC6A76662D1B8AABA },
        { "System.Int32(System.Threading.Timer,System.Threading.Timer)", (Il2CppMethodPointer)InvokeImpl_System_Comparison_601_Invoke_702EFBA29914C6663F211A284A4BD4FF },
        { "System.Int32(System.TimeZoneInfo,System.TimeZoneInfo)", (Il2CppMethodPointer)InvokeImpl_System_Comparison_601_Invoke_EAD6050AF192A6946ED5269EED2D5BDF },
        { "System.Void(System.Object,System.ConsoleCancelEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_ConsoleCancelEventHandler_Invoke_C53BC02A5B1D2EF801D4CDE444794C5A },
        { "System.Void(System.Object,System.EventArgs)", (Il2CppMethodPointer)InvokeImpl_System_EventHandler_Invoke_5D4BFE80FB11C3B5C8380F19CEEF8917 },
        { "System.Void(System.Object,System.Runtime.Serialization.SafeSerializationEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_EventHandler_601_Invoke_0440C66B8C7965B59EB3720CB86F10DE },
        { "System.Void(System.Object,System.Threading.Tasks.UnobservedTaskExceptionEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_EventHandler_601_Invoke_C198F3ECC8382CF718B501AEF39591E9 },
        { "System.Boolean()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_7A3A9394E8B685A6A9BA2D05C7ECCC6E },
        { "System.Collections.Generic.Dictionary`2<System.ValueTuple`2<System.Type,System.String>,System.Runtime.InteropServices.ICustomMarshaler>()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_AEA92837B43BDBFDB5B7AC92A7074CAA },
        { "System.Collections.Generic.KeyValuePair`2<System.Int32,UnityEngine.Vector2>()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_3C4796851B6337FD4C5A04C11B434A07 },
        { "System.String()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_52884981E83073CE4CA2626128392C4C },
        { "System.Text.StringBuilder()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_BF119905D0E9126126C9F177271CCDE7 },
        { "System.Threading.ManualResetEvent()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_4650C80C8345EB36B42C5952D7E24ABE },
        { "System.Threading.SemaphoreSlim()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_322FC716ED9140502D4DB7A31A815477 },
        { "System.Threading.Tasks.Task+ContingentProperties()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_7E5C998D2A63FC7ABF9D1425CC6E355C },
        { "System.String(System.Collections.Generic.KeyValuePair`2<System.String,System.String>)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_51612775A3FD238001795AE7F8774CEF },
        { "System.Boolean(System.Exception)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_3DB697229945861A63E7646435551DAF },
        { "System.Int32(System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_0CE2F8006A37402C00913A791941C2D8 },
        { "System.Boolean(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_C110A6E6DA39CEA93B02EDD35F30097A },
        { "System.Int32(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_7E25632ABFB659591C9C23D4561D7980 },
        { "System.Boolean(System.Reflection.Assembly)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_B7F9C929D2D760F42603214EC9698F6B },
        { "System.Reflection.Assembly(System.Reflection.AssemblyName)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_0E9900F6C7CB2561D49C09C3AE7DE83E },
        { "System.Int32(System.Reflection.MemberInfo)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_E147B2747509D93ACAE5413079059217 },
        { "System.Boolean(System.Reflection.MethodInfo)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_51CEB3E46CAE6B3DF8A69AD83F729605 },
        { "System.Reflection.MemberInfo[](System.Runtime.Serialization.MemberHolder)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_E0C21A67BCFC4A75E3A1F30BCAA86CE3 },
        { "System.Object(System.String)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_A30B508EC9E91D00EC90262FFC23435D },
        { "System.Boolean(System.Type)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_FB46503A990AD6F76C9A5BA91E43FDE4 },
        { "System.Runtime.Serialization.SerializationEvents(System.Type)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_8DE427C01B2A998665357D4AA354A544 },
        { "System.Boolean(UnityEngine.Networking.PlayerConnection.PlayerEditorConnectionEvents+MessageTypeSubscribers)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_441D5010B7E8DFDDCA783A8C59F48C33 },
        { "System.Boolean(UnityEngineInternal.Input.NativeInputUpdateType)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_F8D59E2D4A6B994D3776790B8428FC8A },
        { "System.Boolean(System.Int32,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_DBA73F3DF7E04ADA6EF414EAB3D99AD6 },
        { "System.Type(System.Type,System.Type[])", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_0AA728C1802D98A07EB17AC99F8EA9E2 },
        { "System.Type(System.Reflection.Assembly,System.String,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Func_604_Invoke_98A41561C238028C04C1ECC7C31D4236 },
        { "System.Boolean(System.Threading.Tasks.Task)", (Il2CppMethodPointer)InvokeImpl_System_Predicate_601_Invoke_3C3D7699BC9A85032411CAD8EFBAB7C3 },
        { "System.Boolean(UnityEngine.Events.BaseInvokableCall)", (Il2CppMethodPointer)InvokeImpl_System_Predicate_601_Invoke_F0ADF08342507F130C6D901BCF1C619A },
        { "System.Boolean(UnityEngine.Terrain)", (Il2CppMethodPointer)InvokeImpl_System_Predicate_601_Invoke_78F859C3DBC2E777ABA61B6A194FE916 },
        { "System.Boolean(System.Reflection.MemberInfo,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Reflection_MemberFilter_Invoke_95ED7EA54716BCDB3AF751AF3B389266 },
        { "System.Boolean(System.Type,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Reflection_TypeFilter_Invoke_F9BFBD61A8A5464849C311B7EC71A29A },
        { "System.Reflection.Assembly(System.Object,System.ResolveEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_ResolveEventHandler_Invoke_63D09AAB8B6FC6A81F37CC26A468C659 },
        { "System.Object(System.Runtime.Remoting.Messaging.Header[])", (Il2CppMethodPointer)InvokeImpl_System_Runtime_Remoting_Messaging_HeaderHandler_Invoke_BA219244F7112FD2556020ECAA9AD21A },
        { "System.Void(System.Runtime.Serialization.StreamingContext)", (Il2CppMethodPointer)InvokeImpl_System_Runtime_Serialization_SerializationEventHandler_Invoke_64EA7307F018701E361E37716F8D55D7 },
        { "System.Void(System.Object,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Threading_WaitOrTimerCallback_Invoke_4F7B35AC9F6CCBABAB79EECD22A326E9 },
        { "System.Void(System.Object,System.UnhandledExceptionEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_UnhandledExceptionEventHandler_Invoke_B09F08BCB4B1BC377C8A356FB27BDC13 },
        { "System.Void(UnityEngine.Networking.PlayerConnection.MessageEventArgs)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_C64FC774213E14E1AC95852F0A5DEF8E },
        { "System.Void(UnityEngine.SceneManagement.Scene)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_9A02FA3FB1EF2ECE1B79DDCC8A04C4AD },
        { "System.Void(UnityEngine.SceneManagement.Scene,UnityEngine.SceneManagement.LoadSceneMode)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_47765518F738B82B22B2C889C2B5F732 },
        { "System.Void(UnityEngine.SceneManagement.Scene,UnityEngine.SceneManagement.Scene)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_DFAEF22D84FB1A5C41A9C4B75A41FD82 },
        { nullptr, nullptr },
    };
} // namespace delegatebridge
} // namespace zlua
