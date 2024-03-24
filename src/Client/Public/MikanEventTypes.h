#pragma once

#include "MikanClientTypes.h"
#include "MikanMathTypes.h"

struct MikanConnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "connected";
};

struct MikanDisconnectedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "disconnected";
};

struct MikanVideoSourceOpenedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceOpened";
};

struct MikanVideoSourceClosedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceClosed";
};

struct MikanVideoSourceNewFrameEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceNewFrame";

	MikanVector3f cameraForward;
	MikanVector3f cameraUp;
	MikanVector3f cameraPosition;
	uint64_t frame;
};

struct MikanVideoSourceAttachmentChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceAttachmentChanged";
};

struct MikanVideoSourceIntrinsicsChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceIntrinsicsChanged";
};

struct MikanVideoSourceModeChangedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "videoSourceModeChanged";
};

struct MikanVRDevicePoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "vrDevicePoseUpdate";

	MikanMatrix4f transform;
	MikanVRDeviceID device_id;
	uint64_t frame;
};

struct MikanVRDeviceListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "vrDeviceListUpdate";
};

struct MikanAnchorPoseUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "anchorPoseUpdate";

	MikanTransform transform;
	MikanSpatialAnchorID anchor_id;
};

struct MikanAnchorListUpdateEvent : public MikanEvent
{
	inline static const std::string k_typeName = "anchorListUpdate";
};

struct MikanScriptMessagePostedEvent : public MikanEvent
{
	inline static const std::string k_typeName = "scriptMessagePosted";

	std::string message;
};
