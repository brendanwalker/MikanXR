#pragma once

#include "MikanAPITypes.h"
#include "MikanCoreTypes_json.h"

#include "nlohmann/json.hpp"

// MikanRequest
inline void to_json(nlohmann::json& j, const MikanRequest& p)
{
	j = nlohmann::json{
		{"requestType", p.requestType},
		{"requestId", p.requestId},
		{"version", p.version}
	};
}
inline void from_json(const nlohmann::json& j, MikanRequest& p)
{
	j.at("requestType").get_to(p.requestType);
	j.at("requestId").get_to(p.requestId);
	j.at("version").get_to(p.version);
}

// MikanResponse
inline void to_json(nlohmann::json& j, const MikanResponse& p)
{
	j = nlohmann::json{
		{"responseType", p.responseType},
		{"requestId", p.requestId},
		{"resultCode", p.resultCode}
	};
}
inline void from_json(const nlohmann::json& j, MikanResponse& p)
{
	j.at("responseType").get_to(p.responseType);
	j.at("requestId").get_to(p.requestId);
	j.at("resultCode").get_to(p.resultCode);
}

// MikanEvent
inline void to_json(nlohmann::json& j, const MikanEvent& p)
{	
	j = nlohmann::json{
		{"eventType", p.eventType}
	};
}
inline void from_json(const nlohmann::json& j, MikanEvent& p)
{
	j.at("eventType").get_to(p.eventType);
}

// MikanClientInfo
inline void to_json(nlohmann::json& j, const MikanClientInfo& p)
{
	j = nlohmann::json{
		{"clientId", p.clientId},
		{"engineName", p.engineName},
		{"engineVersion", p.engineVersion},
		{"applicationName", p.applicationName},
		{"applicationVersion", p.applicationVersion},
		{"xrDeviceName", p.xrDeviceName},
		{"graphicsAPI", p.graphicsAPI},
		{"mikanCoreSdkVersion", p.mikanCoreSdkVersion},
		{"supportsRBG24", p.supportsRGB24},
		{"supportsRBGA32", p.supportsRGBA32},
		{"supportsBGRA32", p.supportsBGRA32},
		{"supportsDepth", p.supportsDepth}
	};
}
inline void from_json(const nlohmann::json& j, MikanClientInfo& p)
{
	j.at("clientId").get_to(p.clientId);
	j.at("engineName").get_to(p.engineName);
	j.at("engineVersion").get_to(p.engineVersion);
	j.at("applicationName").get_to(p.applicationName);
	j.at("applicationVersion").get_to(p.applicationVersion);
	j.at("xrDeviceName").get_to(p.xrDeviceName);
	j.at("graphicsAPI").get_to(p.graphicsAPI);
	j.at("mikanCoreSdkVersion").get_to(p.mikanCoreSdkVersion);
	j.at("supportsRBG24").get_to(p.supportsRGB24);
	j.at("supportsRBGA32").get_to(p.supportsRGBA32);
	j.at("supportsBGRA32").get_to(p.supportsBGRA32);
	j.at("supportsDepth").get_to(p.supportsDepth);
}

// MikanClientInfo
inline void to_json(nlohmann::json& j, const MikanColorRGB& p)
{
	j = nlohmann::json{
		{"r", p.r},
		{"g", p.g},
		{"b", p.b}
	};
}
inline void from_json(const nlohmann::json& j, MikanColorRGB& p)
{
	j.at("r").get_to(p.r);
	j.at("g").get_to(p.g);
	j.at("b").get_to(p.b);
}
