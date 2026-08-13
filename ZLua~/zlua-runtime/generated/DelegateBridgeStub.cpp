#include "DelegateBridgeStub.h"

#include "../lvm/LuaEnv.h"
#include "../marshal/MarshalDefs.h"
#include "../marshal/TypedMarshal.h"
#include "../marshal/ArrayMarshal.h"
#include "../marshal/ObjectMarshal.h"
#include "../marshal/StringMarshal.h"
#include "../marshal/StructMarshal.h"
#include "../marshal/DelegateMarshal.h"
#include "../marshal/MarshalMeta.h"
#include "../marshal/OpaqueValueMarshal.h"
#include "../utils/LuaException.h"
#include "../utils/LuaUtil.h"
#include "../utils/LuaStackGuard.h"

namespace zlua
{
namespace delegatebridge
{
    // NativeArray_601_AB3FC873F8A8D14A08049540DD694F38 Unity.Collections.NativeArray`1<UnityEngine.Plane>
    struct NativeArray_601_AB3FC873F8A8D14A08049540DD694F38 {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // NativeArray_601_BBBF9CFFB57B25A2EC6DD492DCF76987 Unity.Collections.NativeArray`1<UnityEngine.Rendering.CullingSplit>
    struct NativeArray_601_BBBF9CFFB57B25A2EC6DD492DCF76987 {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // Vector3_C8815E3C46FAA9E878AFFF2E09238741 UnityEngine.Vector3
    struct Vector3_C8815E3C46FAA9E878AFFF2E09238741 {
    	float __0; // x
    	float __1; // y
    	float __2; // z
    };
    // LODParameters_80FB2563873A2009D8BBC1C68F2C6732 UnityEngine.Rendering.LODParameters
    struct LODParameters_80FB2563873A2009D8BBC1C68F2C6732 {
    	int32_t __0; // m_IsOrthographic
    	Vector3_C8815E3C46FAA9E878AFFF2E09238741 __1; // m_CameraPosition
    	float __2; // m_FieldOfView
    	float __3; // m_OrthoSize
    	int32_t __4; // m_CameraPixelHeight
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
    // BatchPackedCullingViewID_5DBD7F514B267B30D17880CCD94CB94A UnityEngine.Rendering.BatchPackedCullingViewID
    struct BatchPackedCullingViewID_5DBD7F514B267B30D17880CCD94CB94A {
    	uint64_t __0; // handle
    };
    // BatchCullingContext_F53E08A94F6580371CF24984D199BC21 UnityEngine.Rendering.BatchCullingContext
    struct BatchCullingContext_F53E08A94F6580371CF24984D199BC21 {
    	NativeArray_601_AB3FC873F8A8D14A08049540DD694F38 __0; // cullingPlanes
    	NativeArray_601_BBBF9CFFB57B25A2EC6DD492DCF76987 __1; // cullingSplits
    	LODParameters_80FB2563873A2009D8BBC1C68F2C6732 __2; // lodParameters
    	Matrix4x4_97A5FD20F53F58E0FD54ECAEE08F319B __3; // localToWorldMatrix
    	int32_t __4; // viewType
    	int32_t __5; // projectionType
    	int32_t __6; // cullingFlags
    	BatchPackedCullingViewID_5DBD7F514B267B30D17880CCD94CB94A __7; // viewID
    	uint32_t __8; // cullingLayerMask
    	uint64_t __9; // sceneCullingMask
    	uint8_t __10; // isOrthographic
    	int32_t __11; // receiverPlaneOffset
    	int32_t __12; // receiverPlaneCount
    };
    // NativeArray_601_F1B62074026DD2F7C45235F0F25EAFAE Unity.Collections.NativeArray`1<UnityEngine.Rendering.BatchCullingOutputDrawCommands>
    struct NativeArray_601_F1B62074026DD2F7C45235F0F25EAFAE {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    	int32_t __2; // m_AllocatorLabel
    };
    // BatchCullingOutput_1D5B109FCE358865B27D40944234B75E UnityEngine.Rendering.BatchCullingOutput
    struct BatchCullingOutput_1D5B109FCE358865B27D40944234B75E {
    	NativeArray_601_F1B62074026DD2F7C45235F0F25EAFAE __0; // drawCommands
    };
    // CancellationToken_EC7A533A0C889D046A902BC71469CFBD System.Threading.CancellationToken
    struct CancellationToken_EC7A533A0C889D046A902BC71469CFBD {
    	Il2CppObject* __0; // _source
    };
    // CullingGroupEvent_004A566B0D6F736B6223913FA6B7BF48 UnityEngine.CullingGroupEvent
    struct CullingGroupEvent_004A566B0D6F736B6223913FA6B7BF48 {
    	int32_t __0; // m_Index
    	uint8_t __1; // m_PrevState
    	uint8_t __2; // m_ThisState
    };
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
    // JobHandle_5810AE856240C26F555AB78DF0F80CDF Unity.Jobs.JobHandle
    struct JobHandle_5810AE856240C26F555AB78DF0F80CDF {
    	uint64_t __0; // jobGroup
    	int32_t __1; // version
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
    // NativeArray_601_4C27A3A98DD10295B9A7B4F81703D92F Unity.Collections.NativeArray`1<UnityEngine.Experimental.GlobalIllumination.LightDataGI>
    struct NativeArray_601_4C27A3A98DD10295B9A7B4F81703D92F {
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
    // PhotoCaptureResult_E459804A39103EAB02E8E9739EA27278 UnityEngine.Windows.WebCam.PhotoCapture/PhotoCaptureResult
    struct PhotoCaptureResult_E459804A39103EAB02E8E9739EA27278 {
    	int32_t __0; // resultType
    	int64_t __1; // hResult
    };
    // DateTime_99B78B6C6F120B541DD56A273BF2DDD2 System.DateTime
    struct DateTime_99B78B6C6F120B541DD56A273BF2DDD2 {
    	uint64_t __0; // _dateData
    };
    // TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 System.TimeSpan
    struct TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 {
    	int64_t __0; // _ticks
    };
    // PhraseRecognizedEventArgs_AEAA013F5AF112ED2ADC7736C0A1E445 UnityEngine.Windows.Speech.PhraseRecognizedEventArgs
    struct PhraseRecognizedEventArgs_AEAA013F5AF112ED2ADC7736C0A1E445 {
    	int32_t __0; // confidence
    	Il2CppObject* __1; // semanticMeanings
    	Il2CppObject* __2; // text
    	DateTime_99B78B6C6F120B541DD56A273BF2DDD2 __3; // phraseStartTime
    	TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 __4; // phraseDuration
    };
    // PhysicsScene_7789191B084D38B36003CA2FFB99B451 UnityEngine.PhysicsScene
    struct PhysicsScene_7789191B084D38B36003CA2FFB99B451 {
    	int32_t __0; // m_Handle
    };
    // PlayableGraph_F608B63EE21BCBEFAA4798C91D5A9F6C UnityEngine.Playables.PlayableGraph
    struct PlayableGraph_F608B63EE21BCBEFAA4798C91D5A9F6C {
    	intptr_t __0; // m_Handle
    	uint32_t __1; // m_Version
    };
    // PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C UnityEngine.Playables.PlayableOutputHandle
    struct PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C {
    	intptr_t __0; // m_Handle
    	uint32_t __1; // m_Version
    };
    // PlayableOutput_8950E6AFB914CEB31318402633DDB878 UnityEngine.Playables.PlayableOutput
    struct PlayableOutput_8950E6AFB914CEB31318402633DDB878 {
    	PlayableOutputHandle_4E9716E6E8CC4891EC3F152ED8F4577C __0; // m_Handle
    };
    // Point2D_086B8D836FA9D523F610DBBFA3EAC49D ZLua.Tests.Fixtures.Point2D
    struct Point2D_086B8D836FA9D523F610DBBFA3EAC49D {
    	int32_t __0; // X
    	int32_t __1; // Y
    };
    // ReadOnly_650BA07BBC41F101BE96D78AA7B5A8C1 Unity.Collections.NativeArray`1/ReadOnly<UnityEngine.ContactPairHeader>
    struct ReadOnly_650BA07BBC41F101BE96D78AA7B5A8C1 {
    	intptr_t __0; // m_Buffer
    	int32_t __1; // m_Length
    };
    // ReadWriteParameters_D7C7FE8B30A0979B38EB781512999C1A System.IO.Stream/ReadWriteParameters
    struct ReadWriteParameters_D7C7FE8B30A0979B38EB781512999C1A {
    	Il2CppObject* __0; // Buffer
    	int32_t __1; // Offset
    	int32_t __2; // Count
    };
    // RectInt_F9CAE9ECF608A64B18F18C16E6FC1A12 UnityEngine.RectInt
    struct RectInt_F9CAE9ECF608A64B18F18C16E6FC1A12 {
    	int32_t __0; // m_XMin
    	int32_t __1; // m_YMin
    	int32_t __2; // m_Width
    	int32_t __3; // m_Height
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
    // unitytls_errorstate_8B6DE4DCF45E163E269119C983CF926C Mono.Unity.UnityTls/unitytls_errorstate
    struct unitytls_errorstate_8B6DE4DCF45E163E269119C983CF926C {
    	uint32_t __0; // magic
    	uint32_t __1; // code
    	uint64_t __2; // reserved
    };
    // unitytls_key_ref_215CBF919B6378B66A828178CF31E280 Mono.Unity.UnityTls/unitytls_key_ref
    struct unitytls_key_ref_215CBF919B6378B66A828178CF31E280 {
    	uint64_t __0; // handle
    };
    // unitytls_tlsctx_callbacks_93BB333CF3E72448697BBD5C91F628C1 Mono.Unity.UnityTls/unitytls_tlsctx_callbacks
    struct unitytls_tlsctx_callbacks_93BB333CF3E72448697BBD5C91F628C1 {
    	Il2CppObject* __0; // read
    	Il2CppObject* __1; // write
    	intptr_t __2; // data
    };
    // unitytls_tlsctx_protocolrange_50746D9D5C8F5D41A19E9E39C0E2345F Mono.Unity.UnityTls/unitytls_tlsctx_protocolrange
    struct unitytls_tlsctx_protocolrange_50746D9D5C8F5D41A19E9E39C0E2345F {
    	uint32_t __0; // min
    	uint32_t __1; // max
    };
    // unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 Mono.Unity.UnityTls/unitytls_x509_ref
    struct unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 {
    	uint64_t __0; // handle
    };
    // unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 Mono.Unity.UnityTls/unitytls_x509list_ref
    struct unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 {
    	uint64_t __0; // handle
    };
    // ValueTuple_603_1C8AB44998962ACBB01CF329D415EB4C System.ValueTuple`3<System.Byte[],System.Int32,System.Int32>
    struct ValueTuple_603_1C8AB44998962ACBB01CF329D415EB4C {
    	Il2CppObject* __0; // Item1
    	int32_t __1; // Item2
    	int32_t __2; // Item3
    };
    // ValueTuple_605_551F1F7C1A1AF4DF8C58936E020148D8 System.ValueTuple`5<System.IntPtr,System.Int32,System.IntPtr,System.Int32,System.Boolean>
    struct ValueTuple_605_551F1F7C1A1AF4DF8C58936E020148D8 {
    	intptr_t __0; // Item1
    	int32_t __1; // Item2
    	intptr_t __2; // Item3
    	int32_t __3; // Item4
    	bool __4; // Item5
    };
    // ValueTuple_601_FFD2C882B06AFE81E7E05CE9B4AAA358 System.ValueTuple`1<System.Boolean>
    struct ValueTuple_601_FFD2C882B06AFE81E7E05CE9B4AAA358 {
    	bool __0; // Item1
    };
    // ValueTuple_608_A7C147EED19FF8AFF2A9F6E21C552A42 System.ValueTuple`8<System.IntPtr,System.Int32,System.IntPtr,System.Int32,System.IntPtr,System.Int32,System.Boolean,System.ValueTuple`1<System.Boolean>>
    struct ValueTuple_608_A7C147EED19FF8AFF2A9F6E21C552A42 {
    	intptr_t __0; // Item1
    	int32_t __1; // Item2
    	intptr_t __2; // Item3
    	int32_t __3; // Item4
    	intptr_t __4; // Item5
    	int32_t __5; // Item6
    	bool __6; // Item7
    	ValueTuple_601_FFD2C882B06AFE81E7E05CE9B4AAA358 __7; // Rest
    };
    // Vector4_C2796094C1DE915CAB7A0A45AB577CD3 UnityEngine.Vector4
    struct Vector4_C2796094C1DE915CAB7A0A45AB577CD3 {
    	float __0; // x
    	float __1; // y
    	float __2; // z
    	float __3; // w
    };
    // VFXOutputEventArgs_A3C7FD2AFF134B39596176EF77949446 UnityEngine.VFX.VFXOutputEventArgs
    struct VFXOutputEventArgs_A3C7FD2AFF134B39596176EF77949446 {
    	int32_t __0; // <nameId>k__BackingField
    	Il2CppObject* __1; // <eventAttribute>k__BackingField
    };
    // VideoCaptureResult_E28C5B86B352EC733B7CD2D5D7D39DA9 UnityEngine.Windows.WebCam.VideoCapture/VideoCaptureResult
    struct VideoCaptureResult_E28C5B86B352EC733B7CD2D5D7D39DA9 {
    	int32_t __0; // resultType
    	int64_t __1; // hResult
    };
    // VoidTaskResult_C7B02C83F804C8D671B8B966CE3AE3CA System.Threading.Tasks.VoidTaskResult
    union VoidTaskResult_C7B02C83F804C8D671B8B966CE3AE3CA {
    	struct { char __fieldSize_offsetPadding[1];};
    	struct {
    	};
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
    // YogaSize_42C66AB8D3ED6D613767021DFF58B71F UnityEngine.Yoga.YogaSize
    struct YogaSize_42C66AB8D3ED6D613767021DFF58B71F {
    	float __0; // width
    	float __1; // height
    };
    static int64_t InvokeImpl_Mono_Btls_MonoBtlsBioMono_BioControlFunc_Invoke_13D2D5228A8307908B0C437F8346D93E(Il2CppObject* target, intptr_t __p0, int32_t __p1, int64_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int64_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsBioMono_BioReadFunc_Invoke_E6CC88D04B7314FEE75946DDB40808C4(Il2CppObject* target, intptr_t __p0, intptr_t __p1, int32_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsBioMono_BioWriteFunc_Invoke_3D71CEB20C3AEBCC401454C35F283E6B(Il2CppObject* target, intptr_t __p0, intptr_t __p1, int32_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsSelectCallback_Invoke_B1FCFC4808E206B41D1D1A7904CD527E(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsServerNameCallback_Invoke_72978F1366C1B14A411F90678A518B9F(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsSsl_PrintErrorsCallbackFunc_Invoke_87909853BB4E1B37778410924F4F4C1F(Il2CppObject* target, intptr_t __p0, intptr_t __p1, intptr_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsSslCtx_NativeSelectFunc_Invoke_31F4E2CA1B882DC5409DF59591AF561D(Il2CppObject* target, intptr_t __p0, int32_t __p1, intptr_t __p2, intptr_t __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsSslCtx_NativeServerNameFunc_Invoke_BF707255BDCD470C42F6A7F07C50ECB4(Il2CppObject* target, intptr_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsSslCtx_NativeVerifyFunc_Invoke_7B0964A2681F1E760FC6E2224928C41E(Il2CppObject* target, intptr_t __p0, int32_t __p1, intptr_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsVerifyCallback_Invoke_403676178CD7C92566E7C4BEDDBC9647(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_Mono_Btls_MonoBtlsX509LookupMono_BySubjectFunc_Invoke_DFF6EEE351B0193095B610B9E2E3D3F1(Il2CppObject* target, intptr_t __p0, intptr_t __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_Mono_Math_Prime_PrimalityTest_Invoke_B450FF4626D8A6F3F9FE191126792E06(Il2CppObject* target, Il2CppObject* __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Security_Cryptography_DSAManaged_KeyGeneratedEventHandler_Invoke_708D15E0544CE85A45FD9285D5C88E88(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static Il2CppObject* InvokeImpl_Mono_Security_Interface_MonoLocalCertificateSelectionCallback_Invoke_3B4D76EE842B4F54141B1F234409C088(Il2CppObject* target, Il2CppString* __p0, Il2CppObject* __p1, Il2CppObject* __p2, Il2CppArray* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_Mono_Security_Interface_MonoRemoteCertificateValidationCallback_Invoke_07C3974126C40190AD0A99056B8746E2(Il2CppObject* target, Il2CppString* __p0, Il2CppObject* __p1, Il2CppObject* __p2, int32_t __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static unitytls_errorstate_8B6DE4DCF45E163E269119C983CF926C InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_errorstate_create_t_Invoke_120030F5DC194D21D1213FC731E54423(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        unitytls_errorstate_8B6DE4DCF45E163E269119C983CF926C _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_errorstate_raise_error_t_Invoke_1546C9710F37051188E37124D743C9BB(Il2CppObject* target, void* __p0, uint32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_key_free_t_Invoke_94E21DCEE62989B6C58371EF176BE119(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static unitytls_key_ref_215CBF919B6378B66A828178CF31E280 InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_key_get_ref_t_Invoke_5BBD5D9FC64A34347D39456373BFA48F(Il2CppObject* target, void* __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        unitytls_key_ref_215CBF919B6378B66A828178CF31E280 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void* InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_key_parse_der_t_Invoke_ACAE8D59F693B988C82ECE01EE29B94F(Il2CppObject* target, void* __p0, intptr_t __p1, void* __p2, intptr_t __p3, void* __p4, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        LuaUtil::PCall(L, 5, 1, errfunc);
        void* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_random_generate_bytes_t_Invoke_E0BAF1A48D6EF20A34432F8D85AFAF09(Il2CppObject* target, void* __p0, intptr_t __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void* InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_create_client_t_Invoke_4047F3401583C1A3F18B3BB16DC056F3(Il2CppObject* target, unitytls_tlsctx_protocolrange_50746D9D5C8F5D41A19E9E39C0E2345F __p0, unitytls_tlsctx_callbacks_93BB333CF3E72448697BBD5C91F628C1 __p1, void* __p2, intptr_t __p3, void* __p4, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        LuaUtil::PCall(L, 5, 1, errfunc);
        void* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void* InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_create_server_t_Invoke_FF114345BCAA94B5F6EDF1A3ED070776(Il2CppObject* target, unitytls_tlsctx_protocolrange_50746D9D5C8F5D41A19E9E39C0E2345F __p0, unitytls_tlsctx_callbacks_93BB333CF3E72448697BBD5C91F628C1 __p1, uint64_t __p2, uint64_t __p3, void* __p4, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        LuaUtil::PCall(L, 5, 1, errfunc);
        void* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_free_t_Invoke_7C699F4EAC83C2590E0AFC3877A46ED7(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_get_ciphersuite_t_Invoke_2C887C0CE0FA88B82AF4D4353D1753FD(Il2CppObject* target, void* __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_get_protocol_t_Invoke_D773147CAD98E90B4DB86B3ACF4AB4CC(Il2CppObject* target, void* __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_notify_close_t_Invoke_BA18549FCAD93FC8DF7BE19660B3CFDD(Il2CppObject* target, void* __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_process_handshake_t_Invoke_53160BC08B7142EC88188F5A947BD100(Il2CppObject* target, void* __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static intptr_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_read_t_Invoke_8056E568710A0CAE374514F15650D151(Il2CppObject* target, void* __p0, void* __p1, intptr_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        intptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_server_require_client_authentication_t_Invoke_A71FF37A18BCDE2F3B30E7D6F112292A(Il2CppObject* target, void* __p0, unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_certificate_callback_t_Invoke_95F623F5CA1D7522C3C6CF3BD60A1C22(Il2CppObject* target, void* __p0, Il2CppObject* __p1, void* __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_supported_ciphersuites_t_Invoke_09DB44840F303DCC354F01B0FB0E4868(Il2CppObject* target, void* __p0, void* __p1, intptr_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_trace_callback_t_Invoke_72ED0859DFFAD5FDD6E76CB593DECD20(Il2CppObject* target, void* __p0, Il2CppObject* __p1, void* __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_x509verify_callback_t_Invoke_72E8500A738CCA6BFA62E84C00988150(Il2CppObject* target, void* __p0, Il2CppObject* __p1, void* __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static intptr_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509_export_der_t_Invoke_DAD1FAE9C0E8BFB65E86CC41C8AEACEB(Il2CppObject* target, unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 __p0, void* __p1, intptr_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        intptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_append_der_t_Invoke_C373D140C0987EC023818F479F07850B(Il2CppObject* target, void* __p0, void* __p1, intptr_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_append_t_Invoke_24946BF5A6149352364F39C49F400A4B(Il2CppObject* target, void* __p0, unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void* InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_create_t_Invoke_E6F5BAEFA749D831FB0D609D1318DE7E(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        void* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_free_t_Invoke_EC76C5B257D62994E492B5EA9CC9A870(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_get_ref_t_Invoke_5FCFBDE737A27FF783FFA889F6110E38(Il2CppObject* target, void* __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_get_x509_t_Invoke_24C417DAA6CF9D5406C1604A93977FE6(Il2CppObject* target, unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 __p0, intptr_t __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509verify_default_ca_t_Invoke_BB1B27C16387EC784EC3D8B09A23A7E8(Il2CppObject* target, unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 __p0, void* __p1, intptr_t __p2, Il2CppObject* __p3, void* __p4, void* __p5, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        ctx->paramsMeta[5]->cs2luaWriter(L, &__p5, ctx->paramsMeta[5]);
        LuaUtil::PCall(L, 6, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509verify_explicit_ca_t_Invoke_F29EEFA66E15375AE57B17E6C8DA0750(Il2CppObject* target, unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 __p0, unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 __p1, void* __p2, intptr_t __p3, Il2CppObject* __p4, void* __p5, void* __p6, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        ctx->paramsMeta[5]->cs2luaWriter(L, &__p5, ctx->paramsMeta[5]);
        ctx->paramsMeta[6]->cs2luaWriter(L, &__p6, ctx->paramsMeta[6]);
        LuaUtil::PCall(L, 7, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_certificate_callback_Invoke_D9CEAC7CB68EBD7D0CB1FACC74F3086B(Il2CppObject* target, void* __p0, void* __p1, void* __p2, intptr_t __p3, void* __p4, intptr_t __p5, void* __p6, void* __p7, void* __p8, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        ctx->paramsMeta[5]->cs2luaWriter(L, &__p5, ctx->paramsMeta[5]);
        ctx->paramsMeta[6]->cs2luaWriter(L, &__p6, ctx->paramsMeta[6]);
        ctx->paramsMeta[7]->cs2luaWriter(L, &__p7, ctx->paramsMeta[7]);
        ctx->paramsMeta[8]->cs2luaWriter(L, &__p8, ctx->paramsMeta[8]);
        LuaUtil::PCall(L, 9, 0, errfunc);
    }
    static intptr_t InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_read_callback_Invoke_1EE35C7F418430C166AB9F14EA326BA9(Il2CppObject* target, void* __p0, void* __p1, intptr_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        intptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_trace_callback_Invoke_0199F835BEC9C3D7EEC9FC4BF02CEBD4(Il2CppObject* target, void* __p0, void* __p1, void* __p2, intptr_t __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_x509verify_callback_Invoke_F7332D329C05519A59583E2B2BF06A9E(Il2CppObject* target, void* __p0, unitytls_x509list_ref_E135B3BE58B54425BB73AE2DC61B7A82 __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint32_t InvokeImpl_Mono_Unity_UnityTls_unitytls_x509verify_callback_Invoke_FA6B3ABB9996A899B7A61D0144A89A51(Il2CppObject* target, void* __p0, unitytls_x509_ref_27854223462F43176FE1FBA798F37CC1 __p1, uint32_t __p2, void* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_Action_Invoke_FE05452379D12DAF1D88F5EA570E46C1(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_A39F452680A1B20BC60054BA102E0069(Il2CppObject* target, bool __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_E4BD520A20411B67046F7707575812DB(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_24C629DDF46CAFAEE7C2BD96E856EDE1(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_EC0CA2AB4907826B3CFB12E5D3146419(Il2CppObject* target, intptr_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_41AECDF51E0628BC523C0BDA09BE7ED6(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_01C3B4E91E7BB8D2AE1C2BDEDC92F34A(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_98BCD4B5FE4794F54659567EA6CA69FD(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_B2564D4413568352F7BB04494AEA8B1E(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_DF2502038B158F7F1E02921CE830E0C5(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_21B1183073D1D88BE363D7A4693A5F13(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_FAE35AC2BBB990785B5FDA1BC3BC930A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_064555E25A2D5A3C2C6A4CA5217E9DCE(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_F67912BC361F8316BBB0C3057B44A207(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_B639C805DE04B9BE1BA03C6B092DC327(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_664DF8A1C001B12121AE3597F333BA19(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_D4CC203295D7E726DA4F9DABD3631060(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_65DB44D8166DC7F146B450887F064A0C(Il2CppObject* target, TransformDispatchData_DB423FDFE6DDD2E3125F1494BAFA4255 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_DE2F96FFD1B2C72B8D459E162B5CF4DD(Il2CppObject* target, TypeDispatchData_9A31C2DC5F05F365A3887692758FE3DF __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_35B9D8C50646D294D7DBC6F35B5E3F19(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_F61EBB59F65CD90B82969A9B33374E2B(Il2CppObject* target, VFXOutputEventArgs_A3C7FD2AFF134B39596176EF77949446 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_9F21937674EEF54E1104B39CE5F2ED3C(Il2CppObject* target, InputDevice_48A4DCB3CD89C07D83DC681C7063C920 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_7B77326C059E531A29259A53604148E7(Il2CppObject* target, MeshGenerationResult_CD98C63E8CC2D2E24E030DDD9B94E579 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_CAE58F5D4876726570C2457C2755D331(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_7A5105A2DEB7AB86C313BBD797DA9B65(Il2CppObject* target, XRNodeState_F6DEE909529BF35FD47D4041534B2A53 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_601_Invoke_02DDACFD79937AA4A9FCB1F4C63FCC6C(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_75DB1211162494160E4D21CFCC571CD3(Il2CppObject* target, int32_t __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_59B3143529826F3F27315CD6CCCEE9DD(Il2CppObject* target, int32_t __p0, Il2CppString* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_965A6EF0D0686130905C53D1F68A25CC(Il2CppObject* target, Il2CppString* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_5A9CA2189F1B9B549162F2A75A5B743F(Il2CppObject* target, Il2CppString* __p0, bool __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_61BAF5D906CAE0920A497716E1A0873B(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_EF280BF91C750277597B67406BE9A4C4(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_65245978CE7BBDA1BE40D205152EBB38(Il2CppObject* target, int32_t __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_E09F3BDAD5FBF2E47531DBFDF2C5E6C1(Il2CppObject* target, PhysicsScene_7789191B084D38B36003CA2FFB99B451 __p0, NativeArray_601_D0529AAB3CC1DB21D7B7C43A7EF16D47 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_F1A752D897FFE2047E986D383244CF65(Il2CppObject* target, Il2CppObject* __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_90B5FFACA3DCCC58197939127C1BFCD4(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_04DC3443E818AE8304D85C30904C4693(Il2CppObject* target, Il2CppObject* __p0, NativeArray_601_0184FEB69BECD9A48DCE3C28B22B8DB5 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_602_Invoke_4C9086B0516ECC53926ECF01D04F9F6A(Il2CppObject* target, Il2CppObject* __p0, Il2CppArray* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Action_603_Invoke_DC030962EAB57F8748FFC2C45646B4F6(Il2CppObject* target, bool __p0, bool __p1, int32_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_System_Action_603_Invoke_D52448CCB9CCFBB5C3D3B91010391E01(Il2CppObject* target, Il2CppString* __p0, bool __p1, DebugScreenCapture_D801C2E81C22E70B43AF647AB6D62D90 __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_System_Action_606_Invoke_CE304293C0004693C98A2C0C47E06784(Il2CppObject* target, Il2CppArray* __p0, intptr_t __p1, intptr_t __p2, int32_t __p3, int32_t __p4, Il2CppObject* __p5, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        ctx->paramsMeta[5]->cs2luaWriter(L, &__p5, ctx->paramsMeta[5]);
        LuaUtil::PCall(L, 6, 0, errfunc);
    }
    static void InvokeImpl_System_Action_608_Invoke_068298F64FDBCFBE1B797266D472158A(Il2CppObject* target, intptr_t __p0, intptr_t __p1, intptr_t __p2, intptr_t __p3, intptr_t __p4, intptr_t __p5, int32_t __p6, Il2CppObject* __p7, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        ctx->paramsMeta[5]->cs2luaWriter(L, &__p5, ctx->paramsMeta[5]);
        ctx->paramsMeta[6]->cs2luaWriter(L, &__p6, ctx->paramsMeta[6]);
        ctx->paramsMeta[7]->cs2luaWriter(L, &__p7, ctx->paramsMeta[7]);
        LuaUtil::PCall(L, 8, 0, errfunc);
    }
    static void InvokeImpl_System_AssemblyLoadEventHandler_Invoke_CC41FBF96F7EC3415414DB537C69E874(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Buffers_SpanAction_602_Invoke_02E6B8C8678F6D41ECB756E276A547D3(Il2CppObject* target, Span_601_B60174942194C75AB65CF0728AF98DA1 __p0, ValueTuple_603_1C8AB44998962ACBB01CF329D415EB4C __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Buffers_SpanAction_602_Invoke_DA4779E7F9AD5233758D8ABB5D40171A(Il2CppObject* target, Span_601_B60174942194C75AB65CF0728AF98DA1 __p0, ValueTuple_605_551F1F7C1A1AF4DF8C58936E020148D8 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Buffers_SpanAction_602_Invoke_6D06DAEE16F3B57D22BB6606606391B0(Il2CppObject* target, Span_601_B60174942194C75AB65CF0728AF98DA1 __p0, ValueTuple_608_A7C147EED19FF8AFF2A9F6E21C552A42 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static int32_t InvokeImpl_System_Comparison_601_Invoke_93FD44DD6A387F8EC6A76662D1B8AABA(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Comparison_601_Invoke_702EFBA29914C6663F211A284A4BD4FF(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Comparison_601_Invoke_EAD6050AF192A6946ED5269EED2D5BDF(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Console_WindowsConsole_WindowsCancelHandler_Invoke_79F567D6F54A67ABBB0F6ACF875D802C(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_ConsoleCancelEventHandler_Invoke_C53BC02A5B1D2EF801D4CDE444794C5A(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static bool InvokeImpl_System_DateTimeParse_MatchNumberDelegate_Invoke_D25B01C24390BF46BF3F9B71D20D2BCD(Il2CppObject* target, void* __p0, int32_t __p1, void* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_EventHandler_601_Invoke_25BAA79A0D28FB0E024600ACD2CB85FA(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_EventHandler_601_Invoke_0440C66B8C7965B59EB3720CB86F10DE(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_EventHandler_601_Invoke_C198F3ECC8382CF718B501AEF39591E9(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static bool InvokeImpl_System_Func_601_Invoke_7A3A9394E8B685A6A9BA2D05C7ECCC6E(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_601_Invoke_08BA7D2B273FC59F053C4F66D2767DDC(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint8_t InvokeImpl_System_Func_601_Invoke_F238B3BC6CA421B4E4A8D1EE9346BC28(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        uint8_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint16_t InvokeImpl_System_Func_601_Invoke_C174582BE9E5605F651912D3A73A93CB(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        uint16_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_AEA92837B43BDBFDB5B7AC92A7074CAA(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static KeyValuePair_602_73B408C6BD697DD662918B33E1DC2A02 InvokeImpl_System_Func_601_Invoke_3C4796851B6337FD4C5A04C11B434A07(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        KeyValuePair_602_73B408C6BD697DD662918B33E1DC2A02 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_7B9E112A7D39C460CE4B6D8C7B8516D2(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static double InvokeImpl_System_Func_601_Invoke_6E67253BE42B4676EF9E905DC203D5FA(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        double _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_17AD5E61BBC7B8991931E44F49DD4554(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int16_t InvokeImpl_System_Func_601_Invoke_9C8E6265892AC7CC0F11D2D5CEF6A127(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        int16_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_601_Invoke_CE1A48F0E03E79ADA56CE4BCC0DB74CC(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int64_t InvokeImpl_System_Func_601_Invoke_87592068595AFAEA72CF89B8E28AAD70(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        int64_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static intptr_t InvokeImpl_System_Func_601_Invoke_32AC55BB18CCECB632F7839B152530BF(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        intptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_10CE99C1432F54B8C3346934055A996C(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_F9A622E2D353E6FAEED4A69FE64C212E(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_6B3A8C9EB422DBC425518899308489C3(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int8_t InvokeImpl_System_Func_601_Invoke_458A3D7A8BDF02383B59F9A4E16AB6BA(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        int8_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_1CFE21691D4047B56AD22DEA2926FF50(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static float InvokeImpl_System_Func_601_Invoke_0298D81EE416FA37E50F5A70BE18B93C(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        float _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppString* InvokeImpl_System_Func_601_Invoke_52884981E83073CE4CA2626128392C4C(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppString* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_95ED86983B2A83E9FDA26F38FB1C8B1F(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_BF119905D0E9126126C9F177271CCDE7(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_4650C80C8345EB36B42C5952D7E24ABE(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_322FC716ED9140502D4DB7A31A815477(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_7E5C998D2A63FC7ABF9D1425CC6E355C(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_61A3400E340BF5301371B6312F6B9EF6(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_A56C51E57C38C620F4733CE397B26A65(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_601_Invoke_F40BFB27FC960D664852BA7DEFE46D1A(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint16_t InvokeImpl_System_Func_601_Invoke_275D72AAD8CA705FFEF55AE974B2DB78(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        uint16_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint32_t InvokeImpl_System_Func_601_Invoke_FDF78EC4BEA0DA8F32793F16F3272DD2(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint64_t InvokeImpl_System_Func_601_Invoke_ACD0D1A473B7B5E8687D6C17702D9235(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        uint64_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uintptr_t InvokeImpl_System_Func_601_Invoke_D0C8070BDBFE46DF60D204F6F34C973F(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        uintptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 InvokeImpl_System_Func_601_Invoke_77B32EB0A249F3F1B20E2DF6262DC7C7(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector3_C8815E3C46FAA9E878AFFF2E09238741 InvokeImpl_System_Func_601_Invoke_B29683B70FBF19915BBCDA08D2D1BB71(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector4_C2796094C1DE915CAB7A0A45AB577CD3 InvokeImpl_System_Func_601_Invoke_D6513EA102E57D25FB3046B6E2F2EE5D(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        Vector4_C2796094C1DE915CAB7A0A45AB577CD3 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_601_Invoke_553F32EC73CA812A1BEE1C5EFA3E0809(Il2CppObject* target, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        LuaUtil::PCall(L, 0, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_64EC3F44FBBA78EB604C8C129E2EF13D(Il2CppObject* target, bool __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint8_t InvokeImpl_System_Func_602_Invoke_CBA05311CBD7351753426C17F48D24EF(Il2CppObject* target, uint8_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        uint8_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_602_Invoke_8390DD58E791DDE68B31762180AA057E(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_201169876C792F20E7649D02A17789F8(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint16_t InvokeImpl_System_Func_602_Invoke_DEB27BAA39473021A67582A175FEE698(Il2CppObject* target, uint16_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        uint16_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppString* InvokeImpl_System_Func_602_Invoke_51612775A3FD238001795AE7F8774CEF(Il2CppObject* target, KeyValuePair_602_9237FDD8C09D4CA782AA47606CD393A6 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppString* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static double InvokeImpl_System_Func_602_Invoke_664217E8FF4C6EFFBBD4F8BD4F50DEAC(Il2CppObject* target, double __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        double _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_3DB697229945861A63E7646435551DAF(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_602_Invoke_0B2E9799B3BAA21D5F2D52F6BAF8A246(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_29B22FC1D8C51DE586A52190BFF4E5EA(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int16_t InvokeImpl_System_Func_602_Invoke_E44BD3BDE9A3C8466D8C9965ABD864CF(Il2CppObject* target, int16_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int16_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_0CE2F8006A37402C00913A791941C2D8(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_602_Invoke_A90BFFB0CFD92B9C28BA5F248BE4BEB1(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int64_t InvokeImpl_System_Func_602_Invoke_FE9AB652F02ADE4BA5898A6B70177A69(Il2CppObject* target, int64_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int64_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static intptr_t InvokeImpl_System_Func_602_Invoke_818E2669158DF1498BC06E6BD125E22A(Il2CppObject* target, intptr_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        intptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_9C0A7B36C6C4D460241531468D12ACA9(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_C110A6E6DA39CEA93B02EDD35F30097A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_7E25632ABFB659591C9C23D4561D7980(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_0E9900F6C7CB2561D49C09C3AE7DE83E(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_602_Invoke_E147B2747509D93ACAE5413079059217(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_51CEB3E46CAE6B3DF8A69AD83F729605(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_602_Invoke_E0C21A67BCFC4A75E3A1F30BCAA86CE3(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int8_t InvokeImpl_System_Func_602_Invoke_EEE815CE672C29E4D9EF301F4E3A46D5(Il2CppObject* target, int8_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int8_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static float InvokeImpl_System_Func_602_Invoke_F29D030EB30894EB74943CA241AF172A(Il2CppObject* target, float __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        float _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_A30B508EC9E91D00EC90262FFC23435D(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppString* InvokeImpl_System_Func_602_Invoke_9E90EA40162D21292649356DE5793D37(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppString* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Func_602_Invoke_79DFFF448F6A34FB623DEA38A6B64355(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_64B25B4A63518FC922A10A211411F6E9(Il2CppObject* target, CancellationToken_EC7A533A0C889D046A902BC71469CFBD __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_89C046472D75357C06997EE8DDDF0141(Il2CppObject* target, CancellationToken_EC7A533A0C889D046A902BC71469CFBD __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_FB46503A990AD6F76C9A5BA91E43FDE4(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_602_Invoke_8DE427C01B2A998665357D4AA354A544(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint16_t InvokeImpl_System_Func_602_Invoke_4A5D2328C89A2E646153303A2FEDA3B4(Il2CppObject* target, uint16_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        uint16_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint32_t InvokeImpl_System_Func_602_Invoke_93E8611DFA700388EA3094B416C397EA(Il2CppObject* target, uint32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        uint32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uint64_t InvokeImpl_System_Func_602_Invoke_7F4C04E617628B1892AF2571F3A4BB89(Il2CppObject* target, uint64_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        uint64_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static uintptr_t InvokeImpl_System_Func_602_Invoke_CA3932622DF9FD4A53077CA278D19247(Il2CppObject* target, uintptr_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        uintptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_441D5010B7E8DFDDCA783A8C59F48C33(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static float InvokeImpl_System_Func_602_Invoke_FE2AAA099D67370E01E214AF0A8AE7F1(Il2CppObject* target, Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        float _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 InvokeImpl_System_Func_602_Invoke_CC6AF6F13F1FC5714634FEE7EE21BB68(Il2CppObject* target, Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Vector2_1B15CE1B9CBB86BFFFDE29ECFE10B4F1 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static float InvokeImpl_System_Func_602_Invoke_04A0F06257929369CDB4DB3E96FDF6F1(Il2CppObject* target, Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        float _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector3_C8815E3C46FAA9E878AFFF2E09238741 InvokeImpl_System_Func_602_Invoke_BFFDEDFDC365F459E12AF0CA1EA7A045(Il2CppObject* target, Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static float InvokeImpl_System_Func_602_Invoke_837EE6292452D2428EB8C1E3EF85DBE3(Il2CppObject* target, Vector4_C2796094C1DE915CAB7A0A45AB577CD3 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        float _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector4_C2796094C1DE915CAB7A0A45AB577CD3 InvokeImpl_System_Func_602_Invoke_E03080EE95551FA82A06B21FC42EE17B(Il2CppObject* target, Vector4_C2796094C1DE915CAB7A0A45AB577CD3 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Vector4_C2796094C1DE915CAB7A0A45AB577CD3 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_602_Invoke_F8D59E2D4A6B994D3776790B8428FC8A(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_603_Invoke_D01F570D7FDC60A13A8209BBD8DA4972(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_603_Invoke_41FCC7A1812863ADB119B1D512BFE8E9(Il2CppObject* target, int32_t __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Func_603_Invoke_DBA73F3DF7E04ADA6EF414EAB3D99AD6(Il2CppObject* target, int32_t __p0, intptr_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_Func_603_Invoke_9AEAC6F777741AF3B535E17FF9CCC59C(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static VoidTaskResult_C7B02C83F804C8D671B8B966CE3AE3CA InvokeImpl_System_Func_603_Invoke_DCC91544472B674BA377E9D3AF9D98F7(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        VoidTaskResult_C7B02C83F804C8D671B8B966CE3AE3CA _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_603_Invoke_0AA728C1802D98A07EB17AC99F8EA9E2(Il2CppObject* target, Il2CppObject* __p0, Il2CppArray* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Vector3_C8815E3C46FAA9E878AFFF2E09238741 InvokeImpl_System_Func_603_Invoke_5287FAC21A46ED5A72AAF4E28A9AF1C3(Il2CppObject* target, Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p0, Vector3_C8815E3C46FAA9E878AFFF2E09238741 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        Vector3_C8815E3C46FAA9E878AFFF2E09238741 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_604_Invoke_374EC009D4FB54D8E5075BA7F3A11FEC(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, Il2CppObject* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_604_Invoke_98A41561C238028C04C1ECC7C31D4236(Il2CppObject* target, Il2CppObject* __p0, Il2CppString* __p1, bool __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_604_Invoke_90C1588F75612AF3195862B3CE99A4F9(Il2CppObject* target, Il2CppString* __p0, Il2CppObject* __p1, Il2CppObject* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_605_Invoke_CA7323D73EC4FFA5B6423CB4BE4A4106(Il2CppObject* target, Il2CppObject* __p0, ReadWriteParameters_D7C7FE8B30A0979B38EB781512999C1A __p1, Il2CppObject* __p2, Il2CppObject* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Func_605_Invoke_4EEDAEFAE8AD255E476E124173916FAC(Il2CppObject* target, Il2CppString* __p0, int32_t __p1, Il2CppObject* __p2, Il2CppObject* __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_System_IO_Compression_DeflateStream_ReadMethod_Invoke_711C42DDD122494565859D8E9FEA44F0(Il2CppObject* target, Il2CppArray* __p0, int32_t __p1, int32_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_IO_Compression_DeflateStream_WriteMethod_Invoke_B5B07A3A6E02DEBE95EE8F999C59D5C8(Il2CppObject* target, Il2CppArray* __p0, int32_t __p1, int32_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static bool InvokeImpl_System_IO_Enumeration_FileSystemEnumerable_601_FindPredicate_Invoke_B6389EA083A653FBE39D941F63740C5A(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppString* InvokeImpl_System_IO_Enumeration_FileSystemEnumerable_601_FindTransform_Invoke_F290AB076EFAE00DFD06263C5ED6020B(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppString* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_IOAsyncCallback_Invoke_A419810A635AAAC02CE333502A097F5F(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static Il2CppObject* InvokeImpl_System_Net_BindIPEndPoint_Invoke_F6A63FF734CB3688AAD74A945428B116(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, int32_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Net_Dns_GetHostAddressesCallback_Invoke_8CD6D08FBAE18DCAD546258A195C4A27(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppArray* InvokeImpl_System_Net_HeaderParser_Invoke_01CB7C9557AB13B3F1A59CA71E802942(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppArray* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_Net_HttpContinueDelegate_Invoke_E0E81FDBBADD5647E4C9D6B2AB6F50C6(Il2CppObject* target, int32_t __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static Il2CppObject* InvokeImpl_System_Net_Security_LocalCertificateSelectionCallback_Invoke_B898AB728D836A1DE055FDF9142118DC(Il2CppObject* target, Il2CppObject* __p0, Il2CppString* __p1, Il2CppObject* __p2, Il2CppObject* __p3, Il2CppArray* __p4, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        LuaUtil::PCall(L, 5, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Net_Security_RemoteCertificateValidationCallback_Invoke_79089BDCFEA10DBC99C57B26D0D206E6(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, Il2CppObject* __p2, int32_t __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Net_Security_ServerCertSelectionCallback_Invoke_5CE959A7BCEF20A92731E6A80C9FF1D6(Il2CppObject* target, Il2CppString* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_Net_TimerThread_Callback_Invoke_0411F625DB3E27D241DD9ECA4134B1FC(Il2CppObject* target, Il2CppObject* __p0, int32_t __p1, Il2CppObject* __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static bool InvokeImpl_System_Predicate_601_Invoke_3C3D7699BC9A85032411CAD8EFBAB7C3(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Predicate_601_Invoke_F0ADF08342507F130C6D901BCF1C619A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Predicate_601_Invoke_78F859C3DBC2E777ABA61B6A194FE916(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_Reflection_EventInfo_AddEventAdapter_Invoke_74917E1E7182C7AABD7D1C722A0525F3(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static bool InvokeImpl_System_Reflection_MemberFilter_Invoke_95ED7EA54716BCDB3AF751AF3B389266(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Reflection_RuntimePropertyInfo_GetterAdapter_Invoke_B547A09E5CC0667935EFE702E3A9529A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static bool InvokeImpl_System_Reflection_TypeFilter_Invoke_F9BFBD61A8A5464849C311B7EC71A29A(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        bool _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_ResolveEventHandler_Invoke_63D09AAB8B6FC6A81F37CC26A468C659(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Runtime_CompilerServices_ConditionalWeakTable_602_CreateValueCallback_Invoke_97C28F5ACDFDE136C7D92049CAB88F7E(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Runtime_CompilerServices_ConditionalWeakTable_602_CreateValueCallback_Invoke_73D4895644DA27FDB162C240CFA683B3(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static intptr_t InvokeImpl_System_Runtime_InteropServices_Marshal_SecureStringAllocator_Invoke_8A7E4427CDE16739E6E839AEF36AF764(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        intptr_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 InvokeImpl_System_Runtime_Remoting_Lifetime_Lease_RenewalDelegate_Invoke_2DC871661BCB7D2DF71C7A7DE9064449(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        TimeSpan_544A4226FFEFC0FC7DE9C6060362B909 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static Il2CppObject* InvokeImpl_System_Runtime_Remoting_Messaging_HeaderHandler_Invoke_BA219244F7112FD2556020ECAA9AD21A(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        Il2CppObject* _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_Runtime_Serialization_SerializationEventHandler_Invoke_64EA7307F018701E361E37716F8D55D7(Il2CppObject* target, StreamingContext_9EA1561C462D8798D782A597A7649302 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_System_Threading_WaitOrTimerCallback_Invoke_4F7B35AC9F6CCBABAB79EECD22A326E9(Il2CppObject* target, Il2CppObject* __p0, bool __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_UnhandledExceptionEventHandler_Invoke_B09F08BCB4B1BC377C8A356FB27BDC13(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Xml_Schema_ValidationEventHandler_Invoke_E2BDA61CE1113252817F5EAE10A0D697(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_System_Xml_XmlNodeChangedEventHandler_Invoke_5263DE7B9037AFB9254202D17C333512(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static int32_t InvokeImpl_System_Xml_XmlQualifiedName_HashCodeOfStringDelegate_Invoke_DBC9533FA9D4E49F9AD13F67037237FA(Il2CppObject* target, Il2CppString* __p0, int32_t __p1, int64_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_System_Xml_XmlTextReaderImpl_OnDefaultAttributeUseDelegate_Invoke_0AF90C99D9C8A6A7099049CC89B9DF44(Il2CppObject* target, Il2CppObject* __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Analytics_AnalyticsSessionInfo_SessionStateChanged_Invoke_86ADB9BDD5F8F667BF4A912EF972F895(Il2CppObject* target, int32_t __p0, int64_t __p1, int64_t __p2, bool __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Application_LogCallback_Invoke_29CDB287E627C730F5114773DA6BF075(Il2CppObject* target, Il2CppString* __p0, Il2CppString* __p1, int32_t __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Application_MemoryUsageChangedCallback_Invoke_A61FBA11E920FEE70278832C5BB8C52F(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_AudioClip_PCMReaderCallback_Invoke_F05FA2E877549999E97335BBEF315B79(Il2CppObject* target, Il2CppArray* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_CullingGroup_StateChanged_Invoke_D1224FFC1970CA5D2FED6AE25EAA8F57(Il2CppObject* target, CullingGroupEvent_004A566B0D6F736B6223913FA6B7BF48 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_C64FC774213E14E1AC95852F0A5DEF8E(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_9A02FA3FB1EF2ECE1B79DDCC8A04C4AD(Il2CppObject* target, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_47765518F738B82B22B2C889C2B5F732(Il2CppObject* target, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_DFAEF22D84FB1A5C41A9C4B75A41FD82(Il2CppObject* target, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p0, Scene_E7331A80F14DA8FDE306F5639BD2AD98 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Experimental_Audio_AudioSampleProvider_SampleFramesHandler_Invoke_CD00C6E17D37A99BBF5D0ECCA4CA0EA6(Il2CppObject* target, Il2CppObject* __p0, uint32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Experimental_GlobalIllumination_Lightmapping_RequestLightsDelegate_Invoke_903C92087ED77AB3E8FA01FA1C66CC8E(Il2CppObject* target, Il2CppArray* __p0, NativeArray_601_4C27A3A98DD10295B9A7B4F81703D92F __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Physics_ContactEventDelegate_Invoke_E23243BF93220F8EB1EC8D81F9C55E52(Il2CppObject* target, PhysicsScene_7789191B084D38B36003CA2FFB99B451 __p0, ReadOnly_650BA07BBC41F101BE96D78AA7B5A8C1 __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static PlayableOutput_8950E6AFB914CEB31318402633DDB878 InvokeImpl_UnityEngine_Playables_PlayableBinding_CreateOutputMethod_Invoke_A7E374262A11ABB20AC22FCDCEB87740(Il2CppObject* target, PlayableGraph_F608B63EE21BCBEFAA4798C91D5A9F6C __p0, Il2CppString* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        PlayableOutput_8950E6AFB914CEB31318402633DDB878 _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_UnityEngine_RectTransform_ReapplyDrivenProperties_Invoke_6315B70F52F426B98E75831A0205E4C1(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static JobHandle_5810AE856240C26F555AB78DF0F80CDF InvokeImpl_UnityEngine_Rendering_BatchRendererGroup_OnPerformCulling_Invoke_D7046B0BA0846D38C7D8C3A1E72D787D(Il2CppObject* target, Il2CppObject* __p0, BatchCullingContext_F53E08A94F6580371CF24984D199BC21 __p1, BatchCullingOutput_1D5B109FCE358865B27D40944234B75E __p2, intptr_t __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 1, errfunc);
        JobHandle_5810AE856240C26F555AB78DF0F80CDF _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_UnityEngine_TerrainCallbacks_HeightmapChangedCallback_Invoke_C2B0A4D5990CFA99C1442BD48F0171A9(Il2CppObject* target, Il2CppObject* __p0, RectInt_F9CAE9ECF608A64B18F18C16E6FC1A12 __p1, bool __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_TerrainCallbacks_TextureChangedCallback_Invoke_7CBDEE001DFB45A538E6CD4E178E02C4(Il2CppObject* target, Il2CppObject* __p0, Il2CppString* __p1, RectInt_F9CAE9ECF608A64B18F18C16E6FC1A12 __p2, bool __p3, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        LuaUtil::PCall(L, 4, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Video_VideoPlayer_ErrorEventHandler_Invoke_E117F39FDD18CDCA04EB590FA7694853(Il2CppObject* target, Il2CppObject* __p0, Il2CppString* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Video_VideoPlayer_EventHandler_Invoke_CC146209AFB8EC52949140D6263F1988(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Video_VideoPlayer_FrameReadyEventHandler_Invoke_3DA9301F8B08FC8C2F0ED23B8DF80B3F(Il2CppObject* target, Il2CppObject* __p0, int64_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Video_VideoPlayer_TimeEventHandler_Invoke_C972DE6DD25E1F4765F16B8BEA107D1C(Il2CppObject* target, Il2CppObject* __p0, double __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_Speech_DictationRecognizer_DictationCompletedDelegate_Invoke_888469014E6C75DC03372861AE138B7A(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_Speech_DictationRecognizer_DictationErrorHandler_Invoke_FFB90F66FC7613746123BDDA2281BC46(Il2CppObject* target, Il2CppString* __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_Speech_DictationRecognizer_DictationResultDelegate_Invoke_024D530C91A670705B64E1E3CF881CDF(Il2CppObject* target, Il2CppString* __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_Speech_PhraseRecognitionSystem_ErrorDelegate_Invoke_106E66A87C4D8DAFD62162E4A68933B8(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_Speech_PhraseRecognitionSystem_StatusDelegate_Invoke_B3A037C8DBC29858433E77201DE35570(Il2CppObject* target, int32_t __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_Speech_PhraseRecognizer_PhraseRecognizedDelegate_Invoke_6419EA717F50B96E11904DBD53271C01(Il2CppObject* target, PhraseRecognizedEventArgs_AEAA013F5AF112ED2ADC7736C0A1E445 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_WebCam_PhotoCapture_OnCapturedToDiskCallback_Invoke_90BB17A79373FBE70B2358DE909064E0(Il2CppObject* target, PhotoCaptureResult_E459804A39103EAB02E8E9739EA27278 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_WebCam_PhotoCapture_OnCapturedToMemoryCallback_Invoke_5AF6F743AF067CBAC5CFBE5A0B1B0534(Il2CppObject* target, PhotoCaptureResult_E459804A39103EAB02E8E9739EA27278 __p0, Il2CppObject* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_WebCam_PhotoCapture_OnCaptureResourceCreatedCallback_Invoke_7C6F9834C0E8068F703F4A1647062F24(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_WebCam_VideoCapture_OnStartedRecordingVideoCallback_Invoke_F7D1346974F5C5EF0B8BF55E1DC70812(Il2CppObject* target, VideoCaptureResult_E28C5B86B352EC733B7CD2D5D7D39DA9 __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static void InvokeImpl_UnityEngine_Windows_WebCam_VideoCapture_OnVideoCaptureResourceCreatedCallback_Invoke_127A7B5128F018DA26F8C3C0FCB4EC3A(Il2CppObject* target, Il2CppObject* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 0, errfunc);
    }
    static float InvokeImpl_UnityEngine_Yoga_BaselineFunction_Invoke_966B19C0C34E6A5BAEE8FDC8F589F8E9(Il2CppObject* target, Il2CppObject* __p0, float __p1, float __p2, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        LuaUtil::PCall(L, 3, 1, errfunc);
        float _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static YogaSize_42C66AB8D3ED6D613767021DFF58B71F InvokeImpl_UnityEngine_Yoga_MeasureFunction_Invoke_2BAE251204A4DA52B78AD735174B92E3(Il2CppObject* target, Il2CppObject* __p0, float __p1, int32_t __p2, float __p3, int32_t __p4, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        ctx->paramsMeta[2]->cs2luaWriter(L, &__p2, ctx->paramsMeta[2]);
        ctx->paramsMeta[3]->cs2luaWriter(L, &__p3, ctx->paramsMeta[3]);
        ctx->paramsMeta[4]->cs2luaWriter(L, &__p4, ctx->paramsMeta[4]);
        LuaUtil::PCall(L, 5, 1, errfunc);
        YogaSize_42C66AB8D3ED6D613767021DFF58B71F _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static void InvokeImpl_UnityEngineInternal_Input_NativeUpdateCallback_Invoke_212EE3846012C1895D0E3012F7F0A8E4(Il2CppObject* target, int32_t __p0, void* __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 0, errfunc);
    }
    static int32_t InvokeImpl_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaqueFn_Invoke_423F49D568CB5DD2398DB5602B129303(Il2CppObject* target, Point2D_086B8D836FA9D523F610DBBFA3EAC49D __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefIntFn_Invoke_A906AD20F0C2EE1917A5749C8BF05852(Il2CppObject* target, void* __p0, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        LuaUtil::PCall(L, 1, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    static int32_t InvokeImpl_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueIntFn_Invoke_684A0656C434EE7CFD09ABF2B6D32E81(Il2CppObject* target, void* __p0, int32_t __p1, const MethodInfo* method)
    {
        lua_State* L = LuaEnv::GetStateForInvoke(target);
        OpaqueParameterScope opaqueScope;
        LuaMethod* luaMethod = reinterpret_cast<LuaMethod*>(target);
        const MethodMarshalCtx* ctx = luaMethod->methodMarshalCtx;
        LuaStackGuard guard(L);
        int errfunc = LuaEnv::PushErrorHandler();
        LuaUtil::PushRef(L, luaMethod->funcRef);
        (void)method;
        ctx->paramsMeta[0]->cs2luaWriter(L, &__p0, ctx->paramsMeta[0]);
        ctx->paramsMeta[1]->cs2luaWriter(L, &__p1, ctx->paramsMeta[1]);
        LuaUtil::PCall(L, 2, 1, errfunc);
        int32_t _retval{};
        ctx->retMeta->lua2csWriter(L, -1, &_retval, ctx->retMeta);
        return _retval;
    }
    const DelegateBridgeEntry g_delegateBridges[] = {
        { "System.Int64(System.IntPtr,Mono.Btls.MonoBtlsBioMono+ControlCommand,System.Int64)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsBioMono_BioControlFunc_Invoke_13D2D5228A8307908B0C437F8346D93E },
        { "System.Int32(System.IntPtr,System.IntPtr,System.Int32,System.Int32&)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsBioMono_BioReadFunc_Invoke_E6CC88D04B7314FEE75946DDB40808C4 },
        { "System.Int32(System.IntPtr,System.IntPtr,System.Int32)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsBioMono_BioWriteFunc_Invoke_3D71CEB20C3AEBCC401454C35F283E6B },
        { "System.Int32(System.String[])", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsSelectCallback_Invoke_B1FCFC4808E206B41D1D1A7904CD527E },
        { "System.Int32()", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsServerNameCallback_Invoke_72978F1366C1B14A411F90678A518B9F },
        { "System.Int32(System.IntPtr,System.IntPtr,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsSsl_PrintErrorsCallbackFunc_Invoke_87909853BB4E1B37778410924F4F4C1F },
        { "System.Int32(System.IntPtr,System.Int32,System.IntPtr,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsSslCtx_NativeSelectFunc_Invoke_31F4E2CA1B882DC5409DF59591AF561D },
        { "System.Int32(System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsSslCtx_NativeServerNameFunc_Invoke_BF707255BDCD470C42F6A7F07C50ECB4 },
        { "System.Int32(System.IntPtr,System.Int32,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsSslCtx_NativeVerifyFunc_Invoke_7B0964A2681F1E760FC6E2224928C41E },
        { "System.Int32(Mono.Btls.MonoBtlsX509StoreCtx)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsVerifyCallback_Invoke_403676178CD7C92566E7C4BEDDBC9647 },
        { "System.Int32(System.IntPtr,System.IntPtr,System.IntPtr&)", (Il2CppMethodPointer)InvokeImpl_Mono_Btls_MonoBtlsX509LookupMono_BySubjectFunc_Invoke_DFF6EEE351B0193095B610B9E2E3D3F1 },
        { "System.Boolean(Mono.Math.BigInteger,Mono.Math.Prime.ConfidenceFactor)", (Il2CppMethodPointer)InvokeImpl_Mono_Math_Prime_PrimalityTest_Invoke_B450FF4626D8A6F3F9FE191126792E06 },
        { "System.Void(System.Object,System.EventArgs)", (Il2CppMethodPointer)InvokeImpl_Mono_Security_Cryptography_DSAManaged_KeyGeneratedEventHandler_Invoke_708D15E0544CE85A45FD9285D5C88E88 },
        { "System.Security.Cryptography.X509Certificates.X509Certificate(System.String,System.Security.Cryptography.X509Certificates.X509CertificateCollection,System.Security.Cryptography.X509Certificates.X509Certificate,System.String[])", (Il2CppMethodPointer)InvokeImpl_Mono_Security_Interface_MonoLocalCertificateSelectionCallback_Invoke_3B4D76EE842B4F54141B1F234409C088 },
        { "System.Boolean(System.String,System.Security.Cryptography.X509Certificates.X509Certificate,System.Security.Cryptography.X509Certificates.X509Chain,Mono.Security.Interface.MonoSslPolicyErrors)", (Il2CppMethodPointer)InvokeImpl_Mono_Security_Interface_MonoRemoteCertificateValidationCallback_Invoke_07C3974126C40190AD0A99056B8746E2 },
        { "Mono.Unity.UnityTls+unitytls_errorstate()", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_errorstate_create_t_Invoke_120030F5DC194D21D1213FC731E54423 },
        { "System.Void(Mono.Unity.UnityTls+unitytls_errorstate*,Mono.Unity.UnityTls+unitytls_error_code)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_errorstate_raise_error_t_Invoke_1546C9710F37051188E37124D743C9BB },
        { "System.Void(Mono.Unity.UnityTls+unitytls_key*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_key_free_t_Invoke_94E21DCEE62989B6C58371EF176BE119 },
        { "Mono.Unity.UnityTls+unitytls_key_ref(Mono.Unity.UnityTls+unitytls_key*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_key_get_ref_t_Invoke_5BBD5D9FC64A34347D39456373BFA48F },
        { "Mono.Unity.UnityTls+unitytls_key*(System.Byte*,System.IntPtr,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_key_parse_der_t_Invoke_ACAE8D59F693B988C82ECE01EE29B94F },
        { "System.Void(System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_random_generate_bytes_t_Invoke_E0BAF1A48D6EF20A34432F8D85AFAF09 },
        { "Mono.Unity.UnityTls+unitytls_tlsctx*(Mono.Unity.UnityTls+unitytls_tlsctx_protocolrange,Mono.Unity.UnityTls+unitytls_tlsctx_callbacks,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_create_client_t_Invoke_4047F3401583C1A3F18B3BB16DC056F3 },
        { "Mono.Unity.UnityTls+unitytls_tlsctx*(Mono.Unity.UnityTls+unitytls_tlsctx_protocolrange,Mono.Unity.UnityTls+unitytls_tlsctx_callbacks,System.UInt64,System.UInt64,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_create_server_t_Invoke_FF114345BCAA94B5F6EDF1A3ED070776 },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_free_t_Invoke_7C699F4EAC83C2590E0AFC3877A46ED7 },
        { "Mono.Unity.UnityTls+unitytls_ciphersuite(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_get_ciphersuite_t_Invoke_2C887C0CE0FA88B82AF4D4353D1753FD },
        { "Mono.Unity.UnityTls+unitytls_protocol(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_get_protocol_t_Invoke_D773147CAD98E90B4DB86B3ACF4AB4CC },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_notify_close_t_Invoke_BA18549FCAD93FC8DF7BE19660B3CFDD },
        { "Mono.Unity.UnityTls+unitytls_x509verify_result(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_process_handshake_t_Invoke_53160BC08B7142EC88188F5A947BD100 },
        { "System.IntPtr(Mono.Unity.UnityTls+unitytls_tlsctx*,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_read_t_Invoke_8056E568710A0CAE374514F15650D151 },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_x509list_ref,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_server_require_client_authentication_t_Invoke_A71FF37A18BCDE2F3B30E7D6F112292A },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_tlsctx_certificate_callback,System.Void*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_certificate_callback_t_Invoke_95F623F5CA1D7522C3C6CF3BD60A1C22 },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_ciphersuite*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_supported_ciphersuites_t_Invoke_09DB44840F303DCC354F01B0FB0E4868 },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_tlsctx_trace_callback,System.Void*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_trace_callback_t_Invoke_72ED0859DFFAD5FDD6E76CB593DECD20 },
        { "System.Void(Mono.Unity.UnityTls+unitytls_tlsctx*,Mono.Unity.UnityTls+unitytls_tlsctx_x509verify_callback,System.Void*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_tlsctx_set_x509verify_callback_t_Invoke_72E8500A738CCA6BFA62E84C00988150 },
        { "System.IntPtr(Mono.Unity.UnityTls+unitytls_x509_ref,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509_export_der_t_Invoke_DAD1FAE9C0E8BFB65E86CC41C8AEACEB },
        { "System.Void(Mono.Unity.UnityTls+unitytls_x509list*,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_append_der_t_Invoke_C373D140C0987EC023818F479F07850B },
        { "System.Void(Mono.Unity.UnityTls+unitytls_x509list*,Mono.Unity.UnityTls+unitytls_x509_ref,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_append_t_Invoke_24946BF5A6149352364F39C49F400A4B },
        { "Mono.Unity.UnityTls+unitytls_x509list*(Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_create_t_Invoke_E6F5BAEFA749D831FB0D609D1318DE7E },
        { "System.Void(Mono.Unity.UnityTls+unitytls_x509list*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_free_t_Invoke_EC76C5B257D62994E492B5EA9CC9A870 },
        { "Mono.Unity.UnityTls+unitytls_x509list_ref(Mono.Unity.UnityTls+unitytls_x509list*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_get_ref_t_Invoke_5FCFBDE737A27FF783FFA889F6110E38 },
        { "Mono.Unity.UnityTls+unitytls_x509_ref(Mono.Unity.UnityTls+unitytls_x509list_ref,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509list_get_x509_t_Invoke_24C417DAA6CF9D5406C1604A93977FE6 },
        { "Mono.Unity.UnityTls+unitytls_x509verify_result(Mono.Unity.UnityTls+unitytls_x509list_ref,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_x509verify_callback,System.Void*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509verify_default_ca_t_Invoke_BB1B27C16387EC784EC3D8B09A23A7E8 },
        { "Mono.Unity.UnityTls+unitytls_x509verify_result(Mono.Unity.UnityTls+unitytls_x509list_ref,Mono.Unity.UnityTls+unitytls_x509list_ref,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_x509verify_callback,System.Void*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_interface_struct_unitytls_x509verify_explicit_ca_t_Invoke_F29EEFA66E15375AE57B17E6C8DA0750 },
        { "System.Void(System.Void*,Mono.Unity.UnityTls+unitytls_tlsctx*,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_x509name*,System.IntPtr,Mono.Unity.UnityTls+unitytls_x509list_ref*,Mono.Unity.UnityTls+unitytls_key_ref*,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_certificate_callback_Invoke_D9CEAC7CB68EBD7D0CB1FACC74F3086B },
        { "System.IntPtr(System.Void*,System.Byte*,System.IntPtr,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_read_callback_Invoke_1EE35C7F418430C166AB9F14EA326BA9 },
        { "System.Void(System.Void*,Mono.Unity.UnityTls+unitytls_tlsctx*,System.Byte*,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_trace_callback_Invoke_0199F835BEC9C3D7EEC9FC4BF02CEBD4 },
        { "Mono.Unity.UnityTls+unitytls_x509verify_result(System.Void*,Mono.Unity.UnityTls+unitytls_x509list_ref,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_tlsctx_x509verify_callback_Invoke_F7332D329C05519A59583E2B2BF06A9E },
        { "Mono.Unity.UnityTls+unitytls_x509verify_result(System.Void*,Mono.Unity.UnityTls+unitytls_x509_ref,Mono.Unity.UnityTls+unitytls_x509verify_result,Mono.Unity.UnityTls+unitytls_errorstate*)", (Il2CppMethodPointer)InvokeImpl_Mono_Unity_UnityTls_unitytls_x509verify_callback_Invoke_FA6B3ABB9996A899B7A61D0144A89A51 },
        { "System.Void()", (Il2CppMethodPointer)InvokeImpl_System_Action_Invoke_FE05452379D12DAF1D88F5EA570E46C1 },
        { "System.Void(System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_A39F452680A1B20BC60054BA102E0069 },
        { "System.Void(System.IAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_E4BD520A20411B67046F7707575812DB },
        { "System.Void(System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_24C629DDF46CAFAEE7C2BD96E856EDE1 },
        { "System.Void(System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_EC0CA2AB4907826B3CFB12E5D3146419 },
        { "System.Void(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_41AECDF51E0628BC523C0BDA09BE7ED6 },
        { "System.Void(System.String)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_01C3B4E91E7BB8D2AE1C2BDEDC92F34A },
        { "System.Void(System.Text.StringBuilder)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_98BCD4B5FE4794F54659567EA6CA69FD },
        { "System.Void(System.Threading.Tasks.Task`1<System.Net.IPAddress[]>)", (Il2CppMethodPointer)InvokeImpl_System_Action_601_Invoke_B2564D4413568352F7BB04494AEA8B1E },
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
        { "System.Void(System.Span`1<System.Char>,System.ValueTuple`3<System.Byte[],System.Int32,System.Int32>)", (Il2CppMethodPointer)InvokeImpl_System_Buffers_SpanAction_602_Invoke_02E6B8C8678F6D41ECB756E276A547D3 },
        { "System.Void(System.Span`1<System.Char>,System.ValueTuple`5<System.IntPtr,System.Int32,System.IntPtr,System.Int32,System.Boolean>)", (Il2CppMethodPointer)InvokeImpl_System_Buffers_SpanAction_602_Invoke_DA4779E7F9AD5233758D8ABB5D40171A },
        { "System.Void(System.Span`1<System.Char>,System.ValueTuple`8<System.IntPtr,System.Int32,System.IntPtr,System.Int32,System.IntPtr,System.Int32,System.Boolean,System.ValueTuple`1<System.Boolean>>)", (Il2CppMethodPointer)InvokeImpl_System_Buffers_SpanAction_602_Invoke_6D06DAEE16F3B57D22BB6606606391B0 },
        { "System.Int32(Mono.Globalization.Unicode.Level2Map,Mono.Globalization.Unicode.Level2Map)", (Il2CppMethodPointer)InvokeImpl_System_Comparison_601_Invoke_93FD44DD6A387F8EC6A76662D1B8AABA },
        { "System.Int32(System.Threading.Timer,System.Threading.Timer)", (Il2CppMethodPointer)InvokeImpl_System_Comparison_601_Invoke_702EFBA29914C6663F211A284A4BD4FF },
        { "System.Int32(System.TimeZoneInfo,System.TimeZoneInfo)", (Il2CppMethodPointer)InvokeImpl_System_Comparison_601_Invoke_EAD6050AF192A6946ED5269EED2D5BDF },
        { "System.Boolean(System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Console_WindowsConsole_WindowsCancelHandler_Invoke_79F567D6F54A67ABBB0F6ACF875D802C },
        { "System.Void(System.Object,System.ConsoleCancelEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_ConsoleCancelEventHandler_Invoke_C53BC02A5B1D2EF801D4CDE444794C5A },
        { "System.Boolean(System.__DTString&,System.Int32,System.Int32&)", (Il2CppMethodPointer)InvokeImpl_System_DateTimeParse_MatchNumberDelegate_Invoke_D25B01C24390BF46BF3F9B71D20D2BCD },
        { "System.Void(System.Object,System.Net.Sockets.SocketAsyncEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_EventHandler_601_Invoke_25BAA79A0D28FB0E024600ACD2CB85FA },
        { "System.Void(System.Object,System.Runtime.Serialization.SafeSerializationEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_EventHandler_601_Invoke_0440C66B8C7965B59EB3720CB86F10DE },
        { "System.Void(System.Object,System.Threading.Tasks.UnobservedTaskExceptionEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_EventHandler_601_Invoke_C198F3ECC8382CF718B501AEF39591E9 },
        { "System.Boolean()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_7A3A9394E8B685A6A9BA2D05C7ECCC6E },
        { "System.Byte[]()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_08BA7D2B273FC59F053C4F66D2767DDC },
        { "System.Byte()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_F238B3BC6CA421B4E4A8D1EE9346BC28 },
        { "System.Char()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_C174582BE9E5605F651912D3A73A93CB },
        { "System.Collections.Generic.Dictionary`2<System.ValueTuple`2<System.Type,System.String>,System.Runtime.InteropServices.ICustomMarshaler>()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_AEA92837B43BDBFDB5B7AC92A7074CAA },
        { "System.Collections.Generic.KeyValuePair`2<System.Int32,UnityEngine.Vector2>()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_3C4796851B6337FD4C5A04C11B434A07 },
        { "System.DateTimeParse+MatchNumberDelegate()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_7B9E112A7D39C460CE4B6D8C7B8516D2 },
        { "System.Double()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_6E67253BE42B4676EF9E905DC203D5FA },
        { "System.Globalization.DateTimeFormatInfo()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_17AD5E61BBC7B8991931E44F49DD4554 },
        { "System.Int16()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_9C8E6265892AC7CC0F11D2D5CEF6A127 },
        { "System.Int32[]()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_CE1A48F0E03E79ADA56CE4BCC0DB74CC },
        { "System.Int64()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_87592068595AFAEA72CF89B8E28AAD70 },
        { "System.IntPtr()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_32AC55BB18CCECB632F7839B152530BF },
        { "System.IO.Stream()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_10CE99C1432F54B8C3346934055A996C },
        { "System.Net.Sockets.Socket+AwaitableSocketAsyncEventArgs()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_F9A622E2D353E6FAEED4A69FE64C212E },
        { "System.Net.Sockets.Socket+CachedEventArgs()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_6B3A8C9EB422DBC425518899308489C3 },
        { "System.SByte()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_458A3D7A8BDF02383B59F9A4E16AB6BA },
        { "System.Security.Cryptography.X509Certificates.X509CertificateCollection()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_1CFE21691D4047B56AD22DEA2926FF50 },
        { "System.Single()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_0298D81EE416FA37E50F5A70BE18B93C },
        { "System.String()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_52884981E83073CE4CA2626128392C4C },
        { "System.Text.Encoding()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_95ED86983B2A83E9FDA26F38FB1C8B1F },
        { "System.Text.StringBuilder()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_BF119905D0E9126126C9F177271CCDE7 },
        { "System.Threading.ManualResetEvent()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_4650C80C8345EB36B42C5952D7E24ABE },
        { "System.Threading.SemaphoreSlim()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_322FC716ED9140502D4DB7A31A815477 },
        { "System.Threading.Tasks.Task+ContingentProperties()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_7E5C998D2A63FC7ABF9D1425CC6E355C },
        { "System.Threading.Tasks.Task`1<System.Net.BufferOffsetSize>()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_61A3400E340BF5301371B6312F6B9EF6 },
        { "System.Threading.Tasks.Task`1<System.Net.WebResponse>()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_A56C51E57C38C620F4733CE397B26A65 },
        { "System.Threading.Tasks.Task()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_F40BFB27FC960D664852BA7DEFE46D1A },
        { "System.UInt16()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_275D72AAD8CA705FFEF55AE974B2DB78 },
        { "System.UInt32()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_FDF78EC4BEA0DA8F32793F16F3272DD2 },
        { "System.UInt64()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_ACD0D1A473B7B5E8687D6C17702D9235 },
        { "System.UIntPtr()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_D0C8070BDBFE46DF60D204F6F34C973F },
        { "UnityEngine.Vector2()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_77B32EB0A249F3F1B20E2DF6262DC7C7 },
        { "UnityEngine.Vector3()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_B29683B70FBF19915BBCDA08D2D1BB71 },
        { "UnityEngine.Vector4()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_D6513EA102E57D25FB3046B6E2F2EE5D },
        { "ZLua.Tests.Fixtures.ColorKind()", (Il2CppMethodPointer)InvokeImpl_System_Func_601_Invoke_553F32EC73CA812A1BEE1C5EFA3E0809 },
        { "System.Boolean(System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_64EC3F44FBBA78EB604C8C129E2EF13D },
        { "System.Byte(System.Byte)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_CBA05311CBD7351753426C17F48D24EF },
        { "System.Byte[](System.Byte[])", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_8390DD58E791DDE68B31762180AA057E },
        { "System.Int32(System.Byte[])", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_201169876C792F20E7649D02A17789F8 },
        { "System.Char(System.Char)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_DEB27BAA39473021A67582A175FEE698 },
        { "System.String(System.Collections.Generic.KeyValuePair`2<System.String,System.String>)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_51612775A3FD238001795AE7F8774CEF },
        { "System.Double(System.Double)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_664217E8FF4C6EFFBBD4F8BD4F50DEAC },
        { "System.Boolean(System.Exception)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_3DB697229945861A63E7646435551DAF },
        { "System.Net.IPAddress[](System.IAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_0B2E9799B3BAA21D5F2D52F6BAF8A246 },
        { "System.Net.WebResponse(System.IAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_29B22FC1D8C51DE586A52190BFF4E5EA },
        { "System.Int16(System.Int16)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_E44BD3BDE9A3C8466D8C9965ABD864CF },
        { "System.Int32(System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_0CE2F8006A37402C00913A791941C2D8 },
        { "System.Int32[](System.Int32[])", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_A90BFFB0CFD92B9C28BA5F248BE4BEB1 },
        { "System.Int64(System.Int64)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_FE9AB652F02ADE4BA5898A6B70177A69 },
        { "System.IntPtr(System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_818E2669158DF1498BC06E6BD125E22A },
        { "System.Threading.Tasks.Task(System.IO.Stream)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_9C0A7B36C6C4D460241531468D12ACA9 },
        { "System.Boolean(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_C110A6E6DA39CEA93B02EDD35F30097A },
        { "System.Int32(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_7E25632ABFB659591C9C23D4561D7980 },
        { "System.Reflection.Assembly(System.Reflection.AssemblyName)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_0E9900F6C7CB2561D49C09C3AE7DE83E },
        { "System.Int32(System.Reflection.MemberInfo)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_E147B2747509D93ACAE5413079059217 },
        { "System.Boolean(System.Reflection.MethodInfo)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_51CEB3E46CAE6B3DF8A69AD83F729605 },
        { "System.Reflection.MemberInfo[](System.Runtime.Serialization.MemberHolder)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_E0C21A67BCFC4A75E3A1F30BCAA86CE3 },
        { "System.SByte(System.SByte)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_EEE815CE672C29E4D9EF301F4E3A46D5 },
        { "System.Single(System.Single)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_F29D030EB30894EB74943CA241AF172A },
        { "System.Object(System.String)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_A30B508EC9E91D00EC90262FFC23435D },
        { "System.String(System.String)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_9E90EA40162D21292649356DE5793D37 },
        { "System.String[](System.String[])", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_79DFFF448F6A34FB623DEA38A6B64355 },
        { "System.Threading.Tasks.Task`1<System.Int32>(System.Threading.CancellationToken)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_64B25B4A63518FC922A10A211411F6E9 },
        { "System.Threading.Tasks.Task`1<System.Net.HttpWebResponse>(System.Threading.CancellationToken)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_89C046472D75357C06997EE8DDDF0141 },
        { "System.Boolean(System.Type)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_FB46503A990AD6F76C9A5BA91E43FDE4 },
        { "System.Runtime.Serialization.SerializationEvents(System.Type)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_8DE427C01B2A998665357D4AA354A544 },
        { "System.UInt16(System.UInt16)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_4A5D2328C89A2E646153303A2FEDA3B4 },
        { "System.UInt32(System.UInt32)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_93E8611DFA700388EA3094B416C397EA },
        { "System.UInt64(System.UInt64)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_7F4C04E617628B1892AF2571F3A4BB89 },
        { "System.UIntPtr(System.UIntPtr)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_CA3932622DF9FD4A53077CA278D19247 },
        { "System.Boolean(UnityEngine.Networking.PlayerConnection.PlayerEditorConnectionEvents+MessageTypeSubscribers)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_441D5010B7E8DFDDCA783A8C59F48C33 },
        { "System.Single(UnityEngine.Vector2)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_FE2AAA099D67370E01E214AF0A8AE7F1 },
        { "UnityEngine.Vector2(UnityEngine.Vector2)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_CC6AF6F13F1FC5714634FEE7EE21BB68 },
        { "System.Single(UnityEngine.Vector3)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_04A0F06257929369CDB4DB3E96FDF6F1 },
        { "UnityEngine.Vector3(UnityEngine.Vector3)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_BFFDEDFDC365F459E12AF0CA1EA7A045 },
        { "System.Single(UnityEngine.Vector4)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_837EE6292452D2428EB8C1E3EF85DBE3 },
        { "UnityEngine.Vector4(UnityEngine.Vector4)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_E03080EE95551FA82A06B21FC42EE17B },
        { "System.Boolean(UnityEngineInternal.Input.NativeInputUpdateType)", (Il2CppMethodPointer)InvokeImpl_System_Func_602_Invoke_F8D59E2D4A6B994D3776790B8428FC8A },
        { "System.IAsyncResult(System.AsyncCallback,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_D01F570D7FDC60A13A8209BBD8DA4972 },
        { "System.Int32(System.Int32,System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_41FCC7A1812863ADB119B1D512BFE8E9 },
        { "System.Boolean(System.Int32,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_DBA73F3DF7E04ADA6EF414EAB3D99AD6 },
        { "System.Int32(System.IO.Stream,System.IAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_9AEAC6F777741AF3B535E17FF9CCC59C },
        { "System.Threading.Tasks.VoidTaskResult(System.IO.Stream,System.IAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_DCC91544472B674BA377E9D3AF9D98F7 },
        { "System.Type(System.Type,System.Type[])", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_0AA728C1802D98A07EB17AC99F8EA9E2 },
        { "UnityEngine.Vector3(UnityEngine.Vector3,UnityEngine.Vector3)", (Il2CppMethodPointer)InvokeImpl_System_Func_603_Invoke_5287FAC21A46ED5A72AAF4E28A9AF1C3 },
        { "System.IAsyncResult(System.Net.IPEndPoint,System.AsyncCallback,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_604_Invoke_374EC009D4FB54D8E5075BA7F3A11FEC },
        { "System.Type(System.Reflection.Assembly,System.String,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Func_604_Invoke_98A41561C238028C04C1ECC7C31D4236 },
        { "System.IAsyncResult(System.String,System.AsyncCallback,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_604_Invoke_90C1588F75612AF3195862B3CE99A4F9 },
        { "System.IAsyncResult(System.IO.Stream,System.IO.Stream+ReadWriteParameters,System.AsyncCallback,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_605_Invoke_CA7323D73EC4FFA5B6423CB4BE4A4106 },
        { "System.IAsyncResult(System.String,System.Int32,System.AsyncCallback,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Func_605_Invoke_4EEDAEFAE8AD255E476E124173916FAC },
        { "System.Int32(System.Byte[],System.Int32,System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_IO_Compression_DeflateStream_ReadMethod_Invoke_711C42DDD122494565859D8E9FEA44F0 },
        { "System.Void(System.Byte[],System.Int32,System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_IO_Compression_DeflateStream_WriteMethod_Invoke_B5B07A3A6E02DEBE95EE8F999C59D5C8 },
        { "System.Boolean(System.IO.Enumeration.FileSystemEntry&)", (Il2CppMethodPointer)InvokeImpl_System_IO_Enumeration_FileSystemEnumerable_601_FindPredicate_Invoke_B6389EA083A653FBE39D941F63740C5A },
        { "System.String(System.IO.Enumeration.FileSystemEntry&)", (Il2CppMethodPointer)InvokeImpl_System_IO_Enumeration_FileSystemEnumerable_601_FindTransform_Invoke_F290AB076EFAE00DFD06263C5ED6020B },
        { "System.Void(System.IOAsyncResult)", (Il2CppMethodPointer)InvokeImpl_System_IOAsyncCallback_Invoke_A419810A635AAAC02CE333502A097F5F },
        { "System.Net.IPEndPoint(System.Net.ServicePoint,System.Net.IPEndPoint,System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Net_BindIPEndPoint_Invoke_F6A63FF734CB3688AAD74A945428B116 },
        { "System.Net.IPAddress[](System.String)", (Il2CppMethodPointer)InvokeImpl_System_Net_Dns_GetHostAddressesCallback_Invoke_8CD6D08FBAE18DCAD546258A195C4A27 },
        { "System.String[](System.String)", (Il2CppMethodPointer)InvokeImpl_System_Net_HeaderParser_Invoke_01CB7C9557AB13B3F1A59CA71E802942 },
        { "System.Void(System.Int32,System.Net.WebHeaderCollection)", (Il2CppMethodPointer)InvokeImpl_System_Net_HttpContinueDelegate_Invoke_E0E81FDBBADD5647E4C9D6B2AB6F50C6 },
        { "System.Security.Cryptography.X509Certificates.X509Certificate(System.Object,System.String,System.Security.Cryptography.X509Certificates.X509CertificateCollection,System.Security.Cryptography.X509Certificates.X509Certificate,System.String[])", (Il2CppMethodPointer)InvokeImpl_System_Net_Security_LocalCertificateSelectionCallback_Invoke_B898AB728D836A1DE055FDF9142118DC },
        { "System.Boolean(System.Object,System.Security.Cryptography.X509Certificates.X509Certificate,System.Security.Cryptography.X509Certificates.X509Chain,System.Net.Security.SslPolicyErrors)", (Il2CppMethodPointer)InvokeImpl_System_Net_Security_RemoteCertificateValidationCallback_Invoke_79089BDCFEA10DBC99C57B26D0D206E6 },
        { "System.Security.Cryptography.X509Certificates.X509Certificate(System.String)", (Il2CppMethodPointer)InvokeImpl_System_Net_Security_ServerCertSelectionCallback_Invoke_5CE959A7BCEF20A92731E6A80C9FF1D6 },
        { "System.Void(System.Net.TimerThread+Timer,System.Int32,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Net_TimerThread_Callback_Invoke_0411F625DB3E27D241DD9ECA4134B1FC },
        { "System.Boolean(System.Threading.Tasks.Task)", (Il2CppMethodPointer)InvokeImpl_System_Predicate_601_Invoke_3C3D7699BC9A85032411CAD8EFBAB7C3 },
        { "System.Boolean(UnityEngine.Events.BaseInvokableCall)", (Il2CppMethodPointer)InvokeImpl_System_Predicate_601_Invoke_F0ADF08342507F130C6D901BCF1C619A },
        { "System.Boolean(UnityEngine.Terrain)", (Il2CppMethodPointer)InvokeImpl_System_Predicate_601_Invoke_78F859C3DBC2E777ABA61B6A194FE916 },
        { "System.Void(System.Object,System.Delegate)", (Il2CppMethodPointer)InvokeImpl_System_Reflection_EventInfo_AddEventAdapter_Invoke_74917E1E7182C7AABD7D1C722A0525F3 },
        { "System.Boolean(System.Reflection.MemberInfo,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Reflection_MemberFilter_Invoke_95ED7EA54716BCDB3AF751AF3B389266 },
        { "System.Object(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Reflection_RuntimePropertyInfo_GetterAdapter_Invoke_B547A09E5CC0667935EFE702E3A9529A },
        { "System.Boolean(System.Type,System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Reflection_TypeFilter_Invoke_F9BFBD61A8A5464849C311B7EC71A29A },
        { "System.Reflection.Assembly(System.Object,System.ResolveEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_ResolveEventHandler_Invoke_63D09AAB8B6FC6A81F37CC26A468C659 },
        { "Mono.Http.NtlmSession(System.Net.HttpWebRequest)", (Il2CppMethodPointer)InvokeImpl_System_Runtime_CompilerServices_ConditionalWeakTable_602_CreateValueCallback_Invoke_97C28F5ACDFDE136C7D92049CAB88F7E },
        { "System.Threading.OSSpecificSynchronizationContext(System.Object)", (Il2CppMethodPointer)InvokeImpl_System_Runtime_CompilerServices_ConditionalWeakTable_602_CreateValueCallback_Invoke_73D4895644DA27FDB162C240CFA683B3 },
        { "System.IntPtr(System.Int32)", (Il2CppMethodPointer)InvokeImpl_System_Runtime_InteropServices_Marshal_SecureStringAllocator_Invoke_8A7E4427CDE16739E6E839AEF36AF764 },
        { "System.TimeSpan(System.Runtime.Remoting.Lifetime.ILease)", (Il2CppMethodPointer)InvokeImpl_System_Runtime_Remoting_Lifetime_Lease_RenewalDelegate_Invoke_2DC871661BCB7D2DF71C7A7DE9064449 },
        { "System.Object(System.Runtime.Remoting.Messaging.Header[])", (Il2CppMethodPointer)InvokeImpl_System_Runtime_Remoting_Messaging_HeaderHandler_Invoke_BA219244F7112FD2556020ECAA9AD21A },
        { "System.Void(System.Runtime.Serialization.StreamingContext)", (Il2CppMethodPointer)InvokeImpl_System_Runtime_Serialization_SerializationEventHandler_Invoke_64EA7307F018701E361E37716F8D55D7 },
        { "System.Void(System.Object,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_System_Threading_WaitOrTimerCallback_Invoke_4F7B35AC9F6CCBABAB79EECD22A326E9 },
        { "System.Void(System.Object,System.UnhandledExceptionEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_UnhandledExceptionEventHandler_Invoke_B09F08BCB4B1BC377C8A356FB27BDC13 },
        { "System.Void(System.Object,System.Xml.Schema.ValidationEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_Xml_Schema_ValidationEventHandler_Invoke_E2BDA61CE1113252817F5EAE10A0D697 },
        { "System.Void(System.Object,System.Xml.XmlNodeChangedEventArgs)", (Il2CppMethodPointer)InvokeImpl_System_Xml_XmlNodeChangedEventHandler_Invoke_5263DE7B9037AFB9254202D17C333512 },
        { "System.Int32(System.String,System.Int32,System.Int64)", (Il2CppMethodPointer)InvokeImpl_System_Xml_XmlQualifiedName_HashCodeOfStringDelegate_Invoke_DBC9533FA9D4E49F9AD13F67037237FA },
        { "System.Void(System.Xml.IDtdDefaultAttributeInfo,System.Xml.XmlTextReaderImpl)", (Il2CppMethodPointer)InvokeImpl_System_Xml_XmlTextReaderImpl_OnDefaultAttributeUseDelegate_Invoke_0AF90C99D9C8A6A7099049CC89B9DF44 },
        { "System.Void(UnityEngine.Analytics.AnalyticsSessionState,System.Int64,System.Int64,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Analytics_AnalyticsSessionInfo_SessionStateChanged_Invoke_86ADB9BDD5F8F667BF4A912EF972F895 },
        { "System.Void(System.String,System.String,UnityEngine.LogType)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Application_LogCallback_Invoke_29CDB287E627C730F5114773DA6BF075 },
        { "System.Void(UnityEngine.ApplicationMemoryUsageChange& modreq(System.Runtime.InteropServices.InAttribute))", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Application_MemoryUsageChangedCallback_Invoke_A61FBA11E920FEE70278832C5BB8C52F },
        { "System.Void(System.Single[])", (Il2CppMethodPointer)InvokeImpl_UnityEngine_AudioClip_PCMReaderCallback_Invoke_F05FA2E877549999E97335BBEF315B79 },
        { "System.Void(UnityEngine.CullingGroupEvent)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_CullingGroup_StateChanged_Invoke_D1224FFC1970CA5D2FED6AE25EAA8F57 },
        { "System.Void(UnityEngine.Networking.PlayerConnection.MessageEventArgs)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_C64FC774213E14E1AC95852F0A5DEF8E },
        { "System.Void(UnityEngine.SceneManagement.Scene)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_601_Invoke_9A02FA3FB1EF2ECE1B79DDCC8A04C4AD },
        { "System.Void(UnityEngine.SceneManagement.Scene,UnityEngine.SceneManagement.LoadSceneMode)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_47765518F738B82B22B2C889C2B5F732 },
        { "System.Void(UnityEngine.SceneManagement.Scene,UnityEngine.SceneManagement.Scene)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Events_UnityAction_602_Invoke_DFAEF22D84FB1A5C41A9C4B75A41FD82 },
        { "System.Void(UnityEngine.Experimental.Audio.AudioSampleProvider,System.UInt32)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Experimental_Audio_AudioSampleProvider_SampleFramesHandler_Invoke_CD00C6E17D37A99BBF5D0ECCA4CA0EA6 },
        { "System.Void(UnityEngine.Light[],Unity.Collections.NativeArray`1<UnityEngine.Experimental.GlobalIllumination.LightDataGI>)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Experimental_GlobalIllumination_Lightmapping_RequestLightsDelegate_Invoke_903C92087ED77AB3E8FA01FA1C66CC8E },
        { "System.Void(UnityEngine.PhysicsScene,Unity.Collections.NativeArray`1+ReadOnly<UnityEngine.ContactPairHeader>)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Physics_ContactEventDelegate_Invoke_E23243BF93220F8EB1EC8D81F9C55E52 },
        { "UnityEngine.Playables.PlayableOutput(UnityEngine.Playables.PlayableGraph,System.String)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Playables_PlayableBinding_CreateOutputMethod_Invoke_A7E374262A11ABB20AC22FCDCEB87740 },
        { "System.Void(UnityEngine.RectTransform)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_RectTransform_ReapplyDrivenProperties_Invoke_6315B70F52F426B98E75831A0205E4C1 },
        { "Unity.Jobs.JobHandle(UnityEngine.Rendering.BatchRendererGroup,UnityEngine.Rendering.BatchCullingContext,UnityEngine.Rendering.BatchCullingOutput,System.IntPtr)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Rendering_BatchRendererGroup_OnPerformCulling_Invoke_D7046B0BA0846D38C7D8C3A1E72D787D },
        { "System.Void(UnityEngine.Terrain,UnityEngine.RectInt,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_TerrainCallbacks_HeightmapChangedCallback_Invoke_C2B0A4D5990CFA99C1442BD48F0171A9 },
        { "System.Void(UnityEngine.Terrain,System.String,UnityEngine.RectInt,System.Boolean)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_TerrainCallbacks_TextureChangedCallback_Invoke_7CBDEE001DFB45A538E6CD4E178E02C4 },
        { "System.Void(UnityEngine.Video.VideoPlayer,System.String)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Video_VideoPlayer_ErrorEventHandler_Invoke_E117F39FDD18CDCA04EB590FA7694853 },
        { "System.Void(UnityEngine.Video.VideoPlayer)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Video_VideoPlayer_EventHandler_Invoke_CC146209AFB8EC52949140D6263F1988 },
        { "System.Void(UnityEngine.Video.VideoPlayer,System.Int64)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Video_VideoPlayer_FrameReadyEventHandler_Invoke_3DA9301F8B08FC8C2F0ED23B8DF80B3F },
        { "System.Void(UnityEngine.Video.VideoPlayer,System.Double)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Video_VideoPlayer_TimeEventHandler_Invoke_C972DE6DD25E1F4765F16B8BEA107D1C },
        { "System.Void(UnityEngine.Windows.Speech.DictationCompletionCause)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_Speech_DictationRecognizer_DictationCompletedDelegate_Invoke_888469014E6C75DC03372861AE138B7A },
        { "System.Void(System.String,System.Int32)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_Speech_DictationRecognizer_DictationErrorHandler_Invoke_FFB90F66FC7613746123BDDA2281BC46 },
        { "System.Void(System.String,UnityEngine.Windows.Speech.ConfidenceLevel)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_Speech_DictationRecognizer_DictationResultDelegate_Invoke_024D530C91A670705B64E1E3CF881CDF },
        { "System.Void(UnityEngine.Windows.Speech.SpeechError)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_Speech_PhraseRecognitionSystem_ErrorDelegate_Invoke_106E66A87C4D8DAFD62162E4A68933B8 },
        { "System.Void(UnityEngine.Windows.Speech.SpeechSystemStatus)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_Speech_PhraseRecognitionSystem_StatusDelegate_Invoke_B3A037C8DBC29858433E77201DE35570 },
        { "System.Void(UnityEngine.Windows.Speech.PhraseRecognizedEventArgs)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_Speech_PhraseRecognizer_PhraseRecognizedDelegate_Invoke_6419EA717F50B96E11904DBD53271C01 },
        { "System.Void(UnityEngine.Windows.WebCam.PhotoCapture+PhotoCaptureResult)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_WebCam_PhotoCapture_OnCapturedToDiskCallback_Invoke_90BB17A79373FBE70B2358DE909064E0 },
        { "System.Void(UnityEngine.Windows.WebCam.PhotoCapture+PhotoCaptureResult,UnityEngine.Windows.WebCam.PhotoCaptureFrame)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_WebCam_PhotoCapture_OnCapturedToMemoryCallback_Invoke_5AF6F743AF067CBAC5CFBE5A0B1B0534 },
        { "System.Void(UnityEngine.Windows.WebCam.PhotoCapture)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_WebCam_PhotoCapture_OnCaptureResourceCreatedCallback_Invoke_7C6F9834C0E8068F703F4A1647062F24 },
        { "System.Void(UnityEngine.Windows.WebCam.VideoCapture+VideoCaptureResult)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_WebCam_VideoCapture_OnStartedRecordingVideoCallback_Invoke_F7D1346974F5C5EF0B8BF55E1DC70812 },
        { "System.Void(UnityEngine.Windows.WebCam.VideoCapture)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Windows_WebCam_VideoCapture_OnVideoCaptureResourceCreatedCallback_Invoke_127A7B5128F018DA26F8C3C0FCB4EC3A },
        { "System.Single(UnityEngine.Yoga.YogaNode,System.Single,System.Single)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Yoga_BaselineFunction_Invoke_966B19C0C34E6A5BAEE8FDC8F589F8E9 },
        { "UnityEngine.Yoga.YogaSize(UnityEngine.Yoga.YogaNode,System.Single,UnityEngine.Yoga.YogaMeasureMode,System.Single,UnityEngine.Yoga.YogaMeasureMode)", (Il2CppMethodPointer)InvokeImpl_UnityEngine_Yoga_MeasureFunction_Invoke_2BAE251204A4DA52B78AD735174B92E3 },
        { "System.Void(UnityEngineInternal.Input.NativeInputUpdateType,UnityEngineInternal.Input.NativeInputEventBuffer*)", (Il2CppMethodPointer)InvokeImpl_UnityEngineInternal_Input_NativeUpdateCallback_Invoke_212EE3846012C1895D0E3012F7F0A8E4 },
        { "System.Int32(ZLua.Tests.Fixtures.Point2D)", (Il2CppMethodPointer)InvokeImpl_ZLua_Tests_Fixtures_LuaInvokeMarshalAsProbe_ReceiveStructOpaqueFn_Invoke_423F49D568CB5DD2398DB5602B129303 },
        { "System.Int32(System.Int32&)", (Il2CppMethodPointer)InvokeImpl_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_BumpRefIntFn_Invoke_A906AD20F0C2EE1917A5749C8BF05852 },
        { "System.Int32(System.Int32&,System.Int32)", (Il2CppMethodPointer)InvokeImpl_ZLua_Tests_Fixtures_OpaqueValueMarshalProbe_RoundtripOpaqueIntFn_Invoke_684A0656C434EE7CFD09ABF2B6D32E81 },
        { nullptr, nullptr },
    };
} // namespace delegatebridge
} // namespace zlua
