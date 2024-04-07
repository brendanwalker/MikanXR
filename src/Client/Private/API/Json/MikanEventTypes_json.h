#pragma once

#include "MikanEventTypes.h"
#include "MikanApiTypes_json.h"
#include "MikanMathTypes_json.h"

#include "nlohmann/json.hpp"

// MikanConnectedEvent
inline void to_json(nlohmann::json& j, const MikanConnectedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanConnectedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanDisconnectedEvent
inline void to_json(nlohmann::json& j, const MikanDisconnectedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanDisconnectedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanVideoSourceOpenedEvent
inline void to_json(nlohmann::json& j, const MikanVideoSourceOpenedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceOpenedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanVideoSourceClosedEvent
inline void to_json(nlohmann::json& j, const MikanVideoSourceClosedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceClosedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanVideoSourceNewFrameEvent
inline void to_json(nlohmann::json& j, const MikanVideoSourceNewFrameEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
	j.update({
		{"cameraForward", p.cameraForward},
		{"cameraUp", p.cameraUp},
		{"cameraPosition", p.cameraPosition},
		{"frame", p.frame}
	});
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceNewFrameEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
	j.at("cameraForward").get_to(p.cameraForward);
	j.at("cameraUp").get_to(p.cameraUp);
	j.at("cameraPosition").get_to(p.cameraPosition);
	j.at("frame").get_to(p.frame);
}

// MikanVideoSourceAttachmentChangedEvent
inline void to_json(nlohmann::json& j, const MikanVideoSourceAttachmentChangedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceAttachmentChangedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanVideoSourceIntrinsicsChangedEvent
inline void to_json(nlohmann::json& j, const MikanVideoSourceIntrinsicsChangedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceIntrinsicsChangedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanVideoSourceModeChangedEvent
inline void to_json(nlohmann::json& j, const MikanVideoSourceModeChangedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanVideoSourceModeChangedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanVRDevicePoseUpdateEvent
inline void to_json(nlohmann::json& j, const MikanVRDevicePoseUpdateEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));

	j.update({
		{"transform", p.transform},
		{"device_id", p.device_id},
		{"frame", p.frame}
	});
}
inline void from_json(const nlohmann::json& j, MikanVRDevicePoseUpdateEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
	j.at("transform").get_to(p.transform);
	j.at("device_id").get_to(p.device_id);
	j.at("frame").get_to(p.frame);
}

// MikanVRDeviceListUpdateEvent
inline void to_json(nlohmann::json& j, const MikanVRDeviceListUpdateEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanVRDeviceListUpdateEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanAnchorPoseUpdateEvent
inline void to_json(nlohmann::json& j, const MikanAnchorPoseUpdateEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));

	nlohmann::json transformJson;
	to_json(transformJson, p.transform);

	j.update({
		{"transform", transformJson},
		{"anchor_id", p.anchor_id}
	});
}
inline void from_json(const nlohmann::json& j, MikanAnchorPoseUpdateEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
	from_json(j.at("transform"), p.transform);
	j.at("anchor_id").get_to(p.anchor_id);
}

// MikanAnchorListUpdateEvent
inline void to_json(nlohmann::json& j, const MikanAnchorListUpdateEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
}
inline void from_json(const nlohmann::json& j, MikanAnchorListUpdateEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
}

// MikanScriptMessagePostedEvent
inline void to_json(nlohmann::json& j, const MikanScriptMessagePostedEvent& p)
{
	nlohmann::to_json(j, static_cast<MikanEvent>(p));
	j.update({
		{"message", p.message}
	});
}
inline void from_json(const nlohmann::json& j, MikanScriptMessagePostedEvent& p)
{
	from_json(j, static_cast<MikanEvent&>(p));
	j.at("message").get_to(p.message);
}