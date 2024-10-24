#pragma once

#include "MikanAPITypes.h"
#include "MikanCoreTypes_json.h"

#include "nlohmann/json.hpp"

// SerializableString
inline void from_json(const nlohmann::json& j, Serialization::String& p)
{
	std::string value;
	j.get_to(value);
	p.setValue(value);
}

// MikanRequest
inline void to_json(nlohmann::json& j, const MikanRequest& p)
{
	j = nlohmann::json{
		{"requestType", p.requestType.getValue()},
		{"requestId", p.requestId},
		{"version", p.version}
	};
}
inline void from_json(const nlohmann::json& j, MikanRequest& p)
{
	from_json(j.at("requestType"), p.requestType);
	j.at("requestId").get_to(p.requestId);
	j.at("version").get_to(p.version);
}

// MikanResponse
inline void to_json(nlohmann::json& j, const MikanResponse& p)
{
	j = nlohmann::json{
		{"responseType", p.responseType.getValue()},
		{"requestId", p.requestId},
		{"resultCode", p.resultCode}
	};
}
inline void from_json(const nlohmann::json& j, MikanResponse& p)
{
	from_json(j.at("responseType"), p.responseType);
	j.at("requestId").get_to(p.requestId);
	j.at("resultCode").get_to(p.resultCode);
}

// MikanEvent
inline void to_json(nlohmann::json& j, const MikanEvent& p)
{	
	j = nlohmann::json{
		{"eventType", p.eventType.getValue()}
	};
}
inline void from_json(const nlohmann::json& j, MikanEvent& p)
{	
	from_json(j.at("eventType"), p.eventType);
}

// MikanClientInfo
inline void to_json(nlohmann::json& j, const MikanClientInfo& p)
{
	j = nlohmann::json{
		{"clientId", p.clientId.getValue()},
		{"engineName", p.engineName.getValue()},
		{"engineVersion", p.engineVersion.getValue()},
		{"applicationName", p.applicationName.getValue()},
		{"applicationVersion", p.applicationVersion.getValue()},
		{"xrDeviceName", p.xrDeviceName.getValue()},
		{"graphicsAPI", p.graphicsAPI},
		{"mikanCoreSdkVersion", p.mikanCoreSdkVersion},
		{"supportsRGB24", p.supportsRGB24},
		{"supportsRGBA32", p.supportsRGBA32},
		{"supportsBGRA32", p.supportsBGRA32},
		{"supportsDepth", p.supportsDepth}
	};
}
inline void from_json(const nlohmann::json& j, MikanClientInfo& p)
{
	from_json(j.at("clientId"), p.clientId);
	from_json(j.at("engineName"), p.engineName);
	from_json(j.at("engineVersion"), p.engineVersion);
	from_json(j.at("applicationName"), p.applicationName);
	from_json(j.at("applicationVersion"), p.applicationVersion);
	from_json(j.at("xrDeviceName"), p.xrDeviceName);
	j.at("graphicsAPI").get_to(p.graphicsAPI);
	j.at("mikanCoreSdkVersion").get_to(p.mikanCoreSdkVersion);
	j.at("supportsRGB24").get_to(p.supportsRGB24);
	j.at("supportsRGBA32").get_to(p.supportsRGBA32);
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
