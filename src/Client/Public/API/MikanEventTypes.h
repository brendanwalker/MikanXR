#pragma once

#include "MikanAPITypes.h"
#include "MikanMathTypes.h"

struct MikanConnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanConnectedEvent";

	MikanConnectedEvent() : MikanEvent(k_typeName) {}
};

struct MikanDisconnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanDisconnectedEvent";

	MikanDisconnectedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceOpenedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceOpenedEvent";

	MikanVideoSourceOpenedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceClosedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceClosedEvent";

	MikanVideoSourceClosedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceNewFrameEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceNewFrameEvent";

	MikanVector3f cameraForward;
	MikanVector3f cameraUp;
	MikanVector3f cameraPosition;
	uint64_t frame;

	MikanVideoSourceNewFrameEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceAttachmentChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceAttachmentChangedEvent";

	MikanVideoSourceAttachmentChangedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceIntrinsicsChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceIntrinsicsChangedEvent";

	MikanVideoSourceIntrinsicsChangedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceModeChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVideoSourceModeChangedEvent";

	MikanVideoSourceModeChangedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVRDevicePoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVRDevicePoseUpdateEvent";

	MikanMatrix4f transform;
	MikanVRDeviceID device_id;
	uint64_t frame;

	MikanVRDevicePoseUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanVRDeviceListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanVRDeviceListUpdateEvent";

	MikanVRDeviceListUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanAnchorPoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanAnchorPoseUpdateEvent";

	MikanTransform transform;
	MikanSpatialAnchorID anchor_id;

	MikanAnchorPoseUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanAnchorListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanAnchorListUpdateEvent";

	MikanAnchorListUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanScriptMessagePostedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "MikanScriptMessagePostedEvent";

	std::string message;

	MikanScriptMessagePostedEvent() : MikanEvent(k_typeName) {}
};