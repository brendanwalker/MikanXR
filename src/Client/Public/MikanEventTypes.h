#pragma once

#include "MikanClientTypes.h"
#include "MikanMathTypes.h"

struct MikanConnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "connected";

	MikanConnectedEvent() : MikanEvent(k_typeName) {}
};

struct MikanDisconnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "disconnected";

	MikanDisconnectedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceOpenedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceOpened";

	MikanVideoSourceOpenedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceClosedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceClosed";

	MikanVideoSourceClosedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceNewFrameEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceNewFrame";

	MikanVector3f cameraForward;
	MikanVector3f cameraUp;
	MikanVector3f cameraPosition;
	uint64_t frame;

	MikanVideoSourceNewFrameEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceAttachmentChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceAttachmentChanged";

	MikanVideoSourceAttachmentChangedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceIntrinsicsChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceIntrinsicsChanged";

	MikanVideoSourceIntrinsicsChangedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVideoSourceModeChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceModeChanged";

	MikanVideoSourceModeChangedEvent() : MikanEvent(k_typeName) {}
};

struct MikanVRDevicePoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "vrDevicePoseUpdate";

	MikanMatrix4f transform;
	MikanVRDeviceID device_id;
	uint64_t frame;

	MikanVRDevicePoseUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanVRDeviceListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "vrDeviceListUpdate";

	MikanVRDeviceListUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanAnchorPoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "anchorPoseUpdate";

	MikanTransform transform;
	MikanSpatialAnchorID anchor_id;

	MikanAnchorPoseUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanAnchorListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "anchorListUpdate";

	MikanAnchorListUpdateEvent() : MikanEvent(k_typeName) {}
};

struct MikanScriptMessagePostedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "scriptMessagePosted";

	std::string message;

	MikanScriptMessagePostedEvent() : MikanEvent(k_typeName) {}
};
