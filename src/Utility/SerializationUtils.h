#pragma once

#include "stdint.h"

#include <array>
#include <string>
#include <vector>

//-- utility methods -----
namespace SerializationUtils
{
	enum class Endian : int
	{
		Little,
		Big
	};

	Endian get_system_endianness();
	bool wants_byte_flip(Endian desired);

	int16_t read_int16(const uint8_t* inData, Endian desired);
	int32_t read_int32(const uint8_t* inData, Endian desired);
	int32_t read_int24(const uint8_t* inData, Endian desired);
	float read_float(const uint8_t* inData, Endian desired);
	double read_double(const uint8_t* inData, Endian desired);

	void write_int16(uint8_t* outData, int16_t inValue, Endian desired);
	void write_int24(uint8_t* outData, int32_t inValue, Endian desired);
	void write_int32(uint8_t* outData, int32_t inValue, Endian desired);
	void write_float(uint8_t* outData, float inValue, Endian desired);
	void write_double(uint8_t* outData, double inValue, Endian desired);
};

class BinaryWriter
{
public:
	BinaryWriter() = delete;
	BinaryWriter(std::vector<uint8_t>& inBuffer);

	void appendByte(uint8_t value);
	void appendBytes(const uint8_t* byteArray, size_t byteCount);

	template<int Count>
	void appendBytes(const std::array<uint8_t, Count>& byteArray)
	{
		m_buffer.insert(m_buffer.end(), byteArray.begin(), byteArray.end());
	}

private:
	std::vector<uint8_t>& m_buffer;
};

void to_binary(BinaryWriter& writer, bool inValue);
void to_binary(BinaryWriter& writer, int16_t inValue);
void to_binary(BinaryWriter& writer, int32_t inValue);
void to_binary(BinaryWriter& writer, float inValue);
void to_binary(BinaryWriter& writer, double inValue);
void to_binary(BinaryWriter& writer, const std::string& inString);

template<typename T, int Count>
inline void to_binary(BinaryWriter& writer, const std::array<T, Count>& inArray)
{
	to_binary(writer, Count);

	for (const T& value : inArray)
	{
		to_binary(writer, value);
	}
}

template<typename T>
inline void to_binary(BinaryWriter& writer, const std::vector<T>& inVector)
{
	to_binary(writer, (int32_t)inVector.size());

	for (const T& value : inVector)
	{
		to_binary(writer, value);
	}
}