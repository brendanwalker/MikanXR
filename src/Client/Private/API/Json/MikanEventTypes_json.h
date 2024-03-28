#pragma once

#include "MikanEventTypes.h"
#include "MikanMathTypes_json.h"

#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanConnectedEvent, eventType)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanDisconnectedEvent, eventType)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceOpenedEvent, eventType)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceClosedEvent, eventType)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceNewFrameEvent, 
								   eventType, cameraForward, cameraUp, cameraPosition, frame)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceAttachmentChangedEvent, eventType)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceIntrinsicsChangedEvent, eventType)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVideoSourceModeChangedEvent, eventType)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVRDevicePoseUpdateEvent, eventType, transform, device_id, frame)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVRDeviceListUpdateEvent, eventType)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanAnchorPoseUpdateEvent, eventType, transform, anchor_id)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanAnchorListUpdateEvent, eventType)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanScriptMessagePostedEvent, eventType, message)