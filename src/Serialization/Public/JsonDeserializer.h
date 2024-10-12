#pragma once

#include "SerializationVisitor.h"

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Serialization
{
	template<typename t_object_type>
	bool deserializefromJsonString(const std::string& jsonString, t_object_type& instance)
	{
		return deserializefromJsonString(jsonString, &instance, t_object_type::staticGetArchetype());
	}
	bool deserializefromJsonString(const std::string& jsonString, void* instance, rfk::Struct const& structType);

	template<typename t_object_type>
	bool deserializefromJson(const nlohmann::json& jsonObject, t_object_type& instance)
	{
		return deserializefromJson(jsonObject, &instance, t_object_type::staticGetArchetype());
	}
	bool deserializefromJson(const nlohmann::json& jsonObject, void* instance, rfk::Struct const& structType);
};