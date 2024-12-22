#include "SerializableObjectPtr.h"
#include "SerializableObjectPtr.rfks.h"

namespace Serialization
{
	MikanClassId toMikanClassId(RfkClassId classId)
	{
		return *reinterpret_cast<MikanClassId*>(&classId);
	}

	RfkClassId toRfkClassId(MikanClassId classId)
	{
		return *reinterpret_cast<RfkClassId*>(&classId);
	}
}