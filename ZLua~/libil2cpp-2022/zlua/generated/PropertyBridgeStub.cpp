#include "PropertyBridgeStub.h"

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

namespace zlua
{
namespace propertybridge
{
    // CancellationToken_EC7A533A0C889D046A902BC71469CFBD System.Threading.CancellationToken
    struct CancellationToken_EC7A533A0C889D046A902BC71469CFBD {
    	Il2CppObject* __0; // _source
    };
    // Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F UnityEngine.Color
    struct Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F {
    	float __0; // r
    	float __1; // g
    	float __2; // b
    	float __3; // a
    };
    // DateTime_99B78B6C6F120B541DD56A273BF2DDD2 System.DateTime
    struct DateTime_99B78B6C6F120B541DD56A273BF2DDD2 {
    	uint64_t __0; // _dateData
    };
    // Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B UnityEngine.Matrix4x4
    struct Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B {
    	float __0; // m00
    	float __1; // m10
    	float __2; // m20
    	float __3; // m30
    	float __4; // m01
    	float __5; // m11
    	float __6; // m21
    	float __7; // m31
    	float __8; // m02
    	float __9; // m12
    	float __10; // m22
    	float __11; // m32
    	float __12; // m03
    	float __13; // m13
    	float __14; // m23
    	float __15; // m33
    };
    // PhysicsScene_7789191B084D38B36003CA2FFB99B451 UnityEngine.PhysicsScene
    struct PhysicsScene_7789191B084D38B36003CA2FFB99B451 {
    	int32_t __0; // m_Handle
    };
    // PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 UnityEngine.PhysicsScene2D
    struct PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 {
    	int32_t __0; // m_Handle
    };
    // PlayableHandle_B669033953222F22A100400B4FFD646D UnityEngine.Playables.PlayableHandle
    struct PlayableHandle_B669033953222F22A100400B4FFD646D {
    	intptr_t __0; // m_Handle
    	uint32_t __1; // m_Version
    };
    // Playable_C0D666B826280172B12E4B0D528D21CD UnityEngine.Playables.Playable
    struct Playable_C0D666B826280172B12E4B0D528D21CD {
    	PlayableHandle_B669033953222F22A100400B4FFD646D __0; // m_Handle
    };
    // PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C UnityEngine.Playables.PlayableOutputHandle
    struct PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C {
    	intptr_t __0; // m_Handle
    	uint32_t __1; // m_Version
    };
    // ByReference_601_9F3F5FF265A6715FFCFFDEB2AF9E9063 System.ByReference`1<System.Byte>
    struct ByReference_601_9F3F5FF265A6715FFCFFDEB2AF9E9063 {
    	intptr_t __0; // _value
    };
    // ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B System.ReadOnlySpan`1<System.Byte>
    struct ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B {
    	ByReference_601_9F3F5FF265A6715FFCFFDEB2AF9E9063 __0; // _pointer
    	int32_t __1; // _length
    };
    // RegexPrefix_C525D7733549089ECE35E0A62D00C402 System.Text.RegularExpressions.RegexPrefix
    struct RegexPrefix_C525D7733549089ECE35E0A62D00C402 {
    	bool __0; // <CaseInsensitive>k__BackingField
    	Il2CppObject* __1; // <Prefix>k__BackingField
    };
    // TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 System.TimeSpan
    struct TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 {
    	int64_t __0; // _ticks
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
    static void StaticGetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef DateTime_99B78B6C6F120B541DD56A273BF2DDD2 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        DateTime_99B78B6C6F120B541DD56A273BF2DDD2 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(DateTime_99B78B6C6F120B541DD56A273BF2DDD2, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        DateTime_99B78B6C6F120B541DD56A273BF2DDD2 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75(lua_State* L, void* target, const void* ctx)
    {
        typedef DateTime_99B78B6C6F120B541DD56A273BF2DDD2 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        DateTime_99B78B6C6F120B541DD56A273BF2DDD2 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, DateTime_99B78B6C6F120B541DD56A273BF2DDD2, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        DateTime_99B78B6C6F120B541DD56A273BF2DDD2 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A(lua_State* L, void* target, const void* ctx)
    {
        typedef ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        ReadOnlySpan_601_56F0A4C61788679220E702F535D15A0B value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(TimeSpan_544A4226FFEFC0FC7DE9C6060362B909, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F(lua_State* L, void* target, const void* ctx)
    {
        typedef TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, TimeSpan_544A4226FFEFC0FC7DE9C6060362B909, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef RegexPrefix_C525D7733549089ECE35E0A62D00C402 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        RegexPrefix_C525D7733549089ECE35E0A62D00C402 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(RegexPrefix_C525D7733549089ECE35E0A62D00C402, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        RegexPrefix_C525D7733549089ECE35E0A62D00C402 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF(lua_State* L, void* target, const void* ctx)
    {
        typedef RegexPrefix_C525D7733549089ECE35E0A62D00C402 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        RegexPrefix_C525D7733549089ECE35E0A62D00C402 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, RegexPrefix_C525D7733549089ECE35E0A62D00C402, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        RegexPrefix_C525D7733549089ECE35E0A62D00C402 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef CancellationToken_EC7A533A0C889D046A902BC71469CFBD (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        CancellationToken_EC7A533A0C889D046A902BC71469CFBD result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(CancellationToken_EC7A533A0C889D046A902BC71469CFBD, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        CancellationToken_EC7A533A0C889D046A902BC71469CFBD value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738(lua_State* L, void* target, const void* ctx)
    {
        typedef CancellationToken_EC7A533A0C889D046A902BC71469CFBD (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        CancellationToken_EC7A533A0C889D046A902BC71469CFBD result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, CancellationToken_EC7A533A0C889D046A902BC71469CFBD, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        CancellationToken_EC7A533A0C889D046A902BC71469CFBD value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4(lua_State* L, void* target, const void* ctx)
    {
        typedef Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Color_CD6FD0D36B4AB89F59FDAFB0F00D0C0F value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef Vector3_C8815E3C46FAA9E878AFFF2E09238741 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(Vector3_C8815E3C46FAA9E878AFFF2E09238741, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68(lua_State* L, void* target, const void* ctx)
    {
        typedef Vector3_C8815E3C46FAA9E878AFFF2E09238741 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, Vector3_C8815E3C46FAA9E878AFFF2E09238741, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39(lua_State* L, void* target, const void* ctx)
    {
        typedef Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef PhysicsScene_7789191B084D38B36003CA2FFB99B451 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PhysicsScene_7789191B084D38B36003CA2FFB99B451 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(PhysicsScene_7789191B084D38B36003CA2FFB99B451, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PhysicsScene_7789191B084D38B36003CA2FFB99B451 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C(lua_State* L, void* target, const void* ctx)
    {
        typedef PhysicsScene_7789191B084D38B36003CA2FFB99B451 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PhysicsScene_7789191B084D38B36003CA2FFB99B451 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, PhysicsScene_7789191B084D38B36003CA2FFB99B451, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PhysicsScene_7789191B084D38B36003CA2FFB99B451 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322(lua_State* L, void* target, const void* ctx)
    {
        typedef PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PhysicsScene2D_37902B6BAB88062B09A12293C9ACA5D5 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef Playable_C0D666B826280172B12E4B0D528D21CD (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Playable_C0D666B826280172B12E4B0D528D21CD result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(Playable_C0D666B826280172B12E4B0D528D21CD, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Playable_C0D666B826280172B12E4B0D528D21CD value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58(lua_State* L, void* target, const void* ctx)
    {
        typedef Playable_C0D666B826280172B12E4B0D528D21CD (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Playable_C0D666B826280172B12E4B0D528D21CD result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, Playable_C0D666B826280172B12E4B0D528D21CD, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Playable_C0D666B826280172B12E4B0D528D21CD value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef PlayableHandle_B669033953222F22A100400B4FFD646D (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PlayableHandle_B669033953222F22A100400B4FFD646D result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(PlayableHandle_B669033953222F22A100400B4FFD646D, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PlayableHandle_B669033953222F22A100400B4FFD646D value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029(lua_State* L, void* target, const void* ctx)
    {
        typedef PlayableHandle_B669033953222F22A100400B4FFD646D (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PlayableHandle_B669033953222F22A100400B4FFD646D result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, PlayableHandle_B669033953222F22A100400B4FFD646D, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PlayableHandle_B669033953222F22A100400B4FFD646D value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB(lua_State* L, void* target, const void* ctx)
    {
        typedef PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    static void StaticGetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD(lua_State* L, void* target /* nullptr */, const void* ctx)
    {
        typedef Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 (*FnGetter)(const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 result = fnGetter(getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void StaticSetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(value, setter);
    }
    static void InstanceGetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD(lua_State* L, void* target, const void* ctx)
    {
        typedef Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 (*FnGetter)(void*, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        const MethodInfo* getter = marshalCtx->property->get;
        FnGetter fnGetter = (FnGetter)getter->methodPointer;
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 result = fnGetter(target, getter);
        StructMarshal::PushValue(L, &result, marshalCtx->valueTypeKlass, MarshalMeta::EnsureByValMetatableRef(L, marshalCtx->meta));
    }
    static void InstanceSetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD(lua_State* L, void* target, int valueIdx, const void* ctx)
    {
        typedef void (*FnSetter)(void*, Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1, const MethodInfo*);
        const PropertyMarshalCtx* marshalCtx = (const PropertyMarshalCtx*)ctx;
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 value;
        StructMarshal::PopValue(L, valueIdx, marshalCtx->valueTypeKlass, &value);
        const MethodInfo* setter = marshalCtx->property->set;
        FnSetter fnSetter = (FnSetter)setter->methodPointer;
        fnSetter(target, value, setter);
    }
    const PropertyBridgeEntry g_propertyBridges[] = {
        { "System.DateTime", StaticGetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75, StaticSetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75, InstanceGetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75, InstanceSetter_System_DateTime_Now_D4D45B0A41D3FBA0DF9CEA55FD2C5D75, },
        { "System.ReadOnlySpan`1<System.Byte>", StaticGetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A, StaticSetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A, InstanceGetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A, InstanceSetter_System_Globalization_CharUnicodeInfo_CategoriesValue_DBA96C68969B70CEDD7760953A01DC3A, },
        { "System.TimeSpan", StaticGetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F, StaticSetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F, InstanceGetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F, InstanceSetter_System_Runtime_Remoting_Lifetime_LifetimeServices_LeaseManagerPollTime_9C24689F94B93F018F8E4543D67C893F, },
        { "System.Text.RegularExpressions.RegexPrefix", StaticGetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF, StaticSetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF, InstanceGetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF, InstanceSetter_System_Text_RegularExpressions_RegexPrefix_Empty_BBCE946074AB3D8D1CB8445A3C87D3AF, },
        { "System.Threading.CancellationToken", StaticGetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738, StaticSetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738, InstanceGetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738, InstanceSetter_System_Threading_CancellationToken_None_CF37CB91286E2FC8121E6C56038A2738, },
        { "UnityEngine.Color", StaticGetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4, StaticSetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4, InstanceGetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4, InstanceSetter_UnityEngine_Color_red_7253CFE9815B8DD4D344378417C2C2F4, },
        { "UnityEngine.Vector3", StaticGetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68, StaticSetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68, InstanceGetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68, InstanceSetter_UnityEngine_Input_mousePosition_5F293E2F16781387F6A6FB439E958B68, },
        { "UnityEngine.Matrix4x4", StaticGetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39, StaticSetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39, InstanceGetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39, InstanceSetter_UnityEngine_Matrix4x4_identity_D546A6661104869E3EAC6995F781DC39, },
        { "UnityEngine.PhysicsScene", StaticGetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C, StaticSetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C, InstanceGetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C, InstanceSetter_UnityEngine_Physics_defaultPhysicsScene_E1A5FC70B00470BEBF4502F9D71C5A7C, },
        { "UnityEngine.PhysicsScene2D", StaticGetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322, StaticSetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322, InstanceGetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322, InstanceSetter_UnityEngine_Physics2D_defaultPhysicsScene_611F1E1D89ED0E2680B64BBAA40D3322, },
        { "UnityEngine.Playables.Playable", StaticGetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58, StaticSetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58, InstanceGetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58, InstanceSetter_UnityEngine_Playables_Playable_Null_B55A5EA14495BD98B7E4DB9066335D58, },
        { "UnityEngine.Playables.PlayableHandle", StaticGetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029, StaticSetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029, InstanceGetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029, InstanceSetter_UnityEngine_Playables_PlayableHandle_Null_B874D5E8E36D5666F30A0E810E0F5029, },
        { "UnityEngine.Playables.PlayableOutputHandle", StaticGetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB, StaticSetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB, InstanceGetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB, InstanceSetter_UnityEngine_Playables_PlayableOutputHandle_Null_1A2C3083A0D5BABCBCCCB7DBAE8E4ACB, },
        { "UnityEngine.Vector2", StaticGetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD, StaticSetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD, InstanceGetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD, InstanceSetter_UnityEngine_Vector2_zero_60DC0FE14F9567E468D315C6B183D0FD, },
        { nullptr, nullptr, nullptr, nullptr, nullptr },
    };
} // namespace propertybridge
} // namespace zlua
