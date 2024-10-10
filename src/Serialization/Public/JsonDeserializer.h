#pragma once

#include "SerializationVisitor.h"
#include <string>

namespace Serialization
{
	template<typename t_object_type>
	bool deserializefromJsonString(const std::string& jsonString, t_object_type& instance)
	{
		return deserializefromJsonString(jsonString, &instance, t_object_type::staticGetArchetype());
	}
	bool deserializefromJsonString(const std::string& jsonString, void* instance, rfk::Struct const& structType);
};