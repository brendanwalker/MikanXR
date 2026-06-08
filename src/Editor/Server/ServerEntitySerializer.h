#pragma once

#include "IEntityAccessor.h"
#include "SerializationVisitor.h"

namespace Serialization
{
	bool serializeFromEntity(IEntityAccessorConstPtr entityAccessor, void* instance, rfk::Struct const& structType, std::string& outErrorMsg);
};
