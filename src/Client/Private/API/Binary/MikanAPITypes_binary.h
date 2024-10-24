#pragma once

#include "MikanAPITypes.h"
#include "BinaryUtility.h"


inline void to_binary(BinaryWriter& writer, const Serialization::String& s)
{
	to_binary(writer, s.getValue());
}

inline void from_binary(BinaryReader& reader, Serialization::String& s)
{
	std::string value;
	from_binary(reader, value);
	s.setValue(value);
}

inline void to_binary(BinaryWriter& writer, const MikanResponse& r)
{
	to_binary(writer, r.responseType);
	to_binary(writer, r.requestId);
	to_binary(writer, r.resultCode);
}

inline void from_binary(BinaryReader& reader, MikanResponse& r)
{
	from_binary(reader, r.responseType);
	from_binary(reader, r.requestId);
	from_binary(reader, (int32_t &)r.resultCode);
}