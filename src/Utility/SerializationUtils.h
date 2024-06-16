#pragma once

#include "stdint.h"

#include <array>
#include <string>
#include <vector>

//-- utility methods -----
namespace SerializationUtils
{
	bool is_little_endian();
	bool is_big_endian();

	int16_t read_be16int(const uint8_t* inData);
	int32_t read_le32int(const uint8_t* inData);
	int32_t read_be32int(const uint8_t* inData);
	int32_t read_be24int(const uint8_t* inData);
	double read_be64double(const uint8_t* inData);

	void write_be16int(uint8_t* outData, int16_t inValue);
	void write_be24int(uint8_t* outData, int32_t inValue);
	void write_le32int(uint8_t* outData, int32_t inValue);
	void write_be32int(uint8_t* outData, int32_t inValue);
	void write_be64double(uint8_t* outData, double inValue);
};

class BinaryWriter
{
public:
	BinaryWriter() = delete;
	BinaryWriter(std::vector<uint8_t>& inBuffer) : m_buffer(inBuffer) {}

	void writeInt32(int32_t inValue);
	void writeUtf8String(const std::string& inString);

protected:
	template<int Count>
	void appendBytes(const std::array<uint8_t, Count>& byteArray)
	{
		m_buffer.insert(m_buffer.end(), byteArray.begin(), byteArray.end());
	}

	void appendBytes(const uint8_t* byteArray, size_t byteCount)
	{
		if (byteCount > 0)
			m_buffer.insert(m_buffer.end(), byteArray, byteArray + byteCount);
	}

private:
	std::vector<uint8_t>& m_buffer;
};