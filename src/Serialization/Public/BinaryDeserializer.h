#pragma once

#include "SerializationVisitor.h"

#include <vector>

namespace Serialization
{
	template<typename t_object_type>
	bool deserializeFromBytes(const std::vector<uint8_t>& inBytes, t_object_type& instance)
	{
		return deserializeFromJsonString(inBytes, &instance, t_object_type::staticGetArchetype());
	}
	bool deserializeFromJsonString(
		const std::vector<uint8_t>& inBytes,
		void* instance,
		rfk::Struct const& structType);
};