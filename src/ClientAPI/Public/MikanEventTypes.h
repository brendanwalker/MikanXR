#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanEventTypes.rfkh.h"
#endif

struct STRUCT() MikanConnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanConnectedEvent";

	MikanConnectedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanConnectedEvent_GENERATED
	#endif
};

struct STRUCT() MikanDisconnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanDisconnectedEvent";

	MikanDisconnectedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDisconnectedEvent_GENERATED
	#endif
};

struct STRUCT() MikanVideoSourceOpenedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceOpenedEvent";

	MikanVideoSourceOpenedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceOpenedEvent_GENERATED
	#endif
};

struct STRUCT() MikanVideoSourceClosedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceClosedEvent";

	MikanVideoSourceClosedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceClosedEvent_GENERATED
	#endif
};

struct STRUCT() MikanVideoSourceNewFrameEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceNewFrameEvent";

	FIELD()
	MikanVector3f cameraForward;
	FIELD()
	MikanVector3f cameraUp;
	FIELD()
	MikanVector3f cameraPosition;
	FIELD()
	uint64_t frame;

	MikanVideoSourceNewFrameEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceNewFrameEvent_GENERATED
	#endif
};

struct STRUCT() MikanVideoSourceAttachmentChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceAttachmentChangedEvent";

	MikanVideoSourceAttachmentChangedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceAttachmentChangedEvent_GENERATED
	#endif
};

struct STRUCT() MikanVideoSourceIntrinsicsChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceIntrinsicsChangedEvent";

	MikanVideoSourceIntrinsicsChangedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceIntrinsicsChangedEvent_GENERATED
	#endif
};

struct STRUCT() MikanVideoSourceModeChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceModeChangedEvent";

	MikanVideoSourceModeChangedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceModeChangedEvent_GENERATED
	#endif
};

struct STRUCT() MikanVRDevicePoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVRDevicePoseUpdateEvent";

	FIELD()
	MikanMatrix4f transform;
	FIELD()
	MikanVRDeviceID device_id;
	FIELD()
	uint64_t frame;

	MikanVRDevicePoseUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDevicePoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanVRDeviceListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVRDeviceListUpdateEvent";

	MikanVRDeviceListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDeviceListUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanAnchorNameUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanAnchorNameChangeEvent";

	FIELD()
	MikanSpatialAnchorID anchor_id;
	FIELD()
	Serialization::String anchor_name;

	MikanAnchorNameUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorNameUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanAnchorPoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanAnchorPoseUpdateEvent";

	FIELD()
	MikanTransform transform;
	FIELD()
	MikanSpatialAnchorID anchor_id;

	MikanAnchorPoseUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorPoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanAnchorListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanAnchorListUpdateEvent";

	MikanAnchorListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanAnchorListUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanStencilNameUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanStencilNameChangeEvent";

	FIELD()
	MikanStencilID stencil_id;
	FIELD()
	Serialization::String stencil_name;

	MikanStencilNameUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilNameUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanStencilPoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanStencilPoseUpdateEvent";

	FIELD()
	MikanTransform transform;
	FIELD()
	MikanStencilID stencil_id;

	MikanStencilPoseUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilPoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanQuadStencilListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanQuadStencilListUpdateEvent";

	MikanQuadStencilListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanQuadStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanBoxStencilListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanBoxStencilListUpdateEvent";

	MikanBoxStencilListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanBoxStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanModelStencilListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanModelStencilListUpdateEvent";

	MikanModelStencilListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanModelStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT() MikanScriptMessagePostedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanScriptMessagePostedEvent";

	FIELD()
	Serialization::String message;

	MikanScriptMessagePostedEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanScriptMessagePostedEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanEventTypes_GENERATED
#endif