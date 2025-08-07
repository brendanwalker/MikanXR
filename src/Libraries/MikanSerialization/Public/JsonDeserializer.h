#pragma once

#include "SerializationExport.h"
#include "SerializationVisitor.h"

#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Serialization
{
	SERIALIZATION_API bool deserializeFromJsonString(
		const std::string& jsonString, 
		void* instance, 
		rfk::Struct const& structType);

	SERIALIZATION_API bool deserializeFromJson(
		const nlohmann::json& jsonObject, 
		void* instance, 
		rfk::Struct const& structType);

#ifdef SERIALIZATION_REFLECTION_ENABLED
	template<typename t_object_type>
	bool deserializeFromJsonString(const std::string& jsonString, t_object_type& instance)
	{
		return deserializeFromJsonString(jsonString, &instance, t_object_type::staticGetArchetype());
	}

	template<typename t_object_type>
	bool deserializeFromJson(const nlohmann::json& jsonObject, t_object_type& instance)
	{
		return deserializeFromJson(jsonObject, &instance, t_object_type::staticGetArchetype());
	}
#endif // SERIALIZATION_REFLECTION_ENABLED
};
