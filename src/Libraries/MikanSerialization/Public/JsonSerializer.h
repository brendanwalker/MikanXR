#pragma once

#include "SerializationExport.h"
#include "SerializationVisitor.h"

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Serialization
{
	SERIALIZATION_API bool serializeToJsonString(
		const void* instance, rfk::Struct const& structType, std::string& jsonString);

	SERIALIZATION_API bool serializeToJson(
		const void* instance, rfk::Struct const& structType, nlohmann::json& jsonObject);

#ifdef SERIALIZATION_REFLECTION_ENABLED
	template<typename t_object_type>
	bool serializeToJsonString(const t_object_type& instance, std::string& jsonString)
	{
		return serializeToJsonString(&instance, t_object_type::staticGetArchetype(), jsonString);
	}

	template<typename t_object_type>
	bool serializeToJson(const t_object_type& instance, nlohmann::json& jsonObject)
	{
		return serializeToJson(&instance, t_object_type::staticGetArchetype(), jsonObject);
	}
#endif // SERIALIZATION_REFLECTION_ENABLED
};
