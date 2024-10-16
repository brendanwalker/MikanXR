#include "BinaryUtility.h"
#include <cstring>
#include <stdexcept>

namespace Serialization
{
	Endian get_system_endianness()
	{
		const int value{0x01};
		const void* address{static_cast<const void*>(&value)};
		const unsigned char* least_significant_address{static_cast<const unsigned char*>(address)};

		return (*least_significant_address == 0x01) ? Endian::Little : Endian::Big;
	}

	bool wants_byte_flip(Endian desired)
	{
		return desired != get_system_endianness();
	}

	int16_t read_int16(const uint8_t* inData, Endian desired)
	{
		int16_t result;

		if (wants_byte_flip(desired)) 
		{
			result = inData[0] << 8;
			result += inData[1];
		}
		else 
		{
			result = inData[0];
			result += inData[1] << 8;
		}
		return result;
	}


	int32_t read_int24(const uint8_t* inData, Endian desired)
	{
		int32_t result;

		if (wants_byte_flip(desired)) 
		{
			result = inData[0] << 16;
			result += inData[1] << 8;
			result += inData[2];
		}
		else 
		{
			result = inData[0];
			result += inData[1] << 8;
			result += inData[2] << 16;
		}

		return result;
	}

	int32_t read_int32(const uint8_t* inData, Endian desired)
	{
		int32_t result;

		if (wants_byte_flip(desired)) 
		{
			result = inData[0] << 24;
			result += inData[1] << 16;
			result += inData[2] << 8;
			result += inData[3];
		}
		else 
		{
			result = inData[0];
			result += inData[1] << 8;
			result += inData[2] << 16;
			result += inData[3] << 24;
		}
		return result;
	}

	int32_t read_int64(const uint8_t* inData, Endian desired)
	{
		int32_t result;

		if (wants_byte_flip(desired))
		{
			result = inData[0] << 56;
			result += inData[1] << 48;
			result += inData[2] << 40;
			result += inData[3] << 32;
			result += inData[4] << 24;
			result += inData[5] << 16;
			result += inData[6] << 8;
			result += inData[7];
		}
		else
		{
			result = inData[0];
			result += inData[1] << 8;
			result += inData[2] << 16;
			result += inData[3] << 24;
			result += inData[4] << 32;
			result += inData[5] << 40;
			result += inData[6] << 48;
			result += inData[7] << 56;
		}
		return result;
	}

	float read_float(const uint8_t* inData, Endian desired)
	{
		float result;
		uint8_t number_data[4];

		if (wants_byte_flip(desired))
		{
			number_data[0] = inData[3];
			number_data[1] = inData[2];
			number_data[2] = inData[1];
			number_data[3] = inData[0];
			std::memmove(&result, number_data, 4);
		}
		else
		{
			std::memmove(&result, inData, 4);
		}

		return result;
	}

	double read_double(const uint8_t* inData, Endian desired)
	{
		double result;
		uint8_t number_data[8];

		if (wants_byte_flip(desired)) 
		{
			number_data[0] = inData[7];
			number_data[1] = inData[6];
			number_data[2] = inData[5];
			number_data[3] = inData[4];
			number_data[4] = inData[3];
			number_data[5] = inData[2];
			number_data[6] = inData[1];
			number_data[7] = inData[0];
			std::memmove(&result, number_data, 8);
		}
		else 
		{
			std::memmove(&result, inData, 8);
		}

		return result;
	}

	void write_float(uint8_t* data, float inValue, Endian desired)
	{
		uint8_t number_data[4];

		if (wants_byte_flip(desired))
		{
			std::memmove(number_data, &inValue, 4);
			data[0] = number_data[3];
			data[1] = number_data[2];
			data[2] = number_data[1];
			data[3] = number_data[0];
		}
		else
		{
			std::memmove(data, &inValue, 4);
		}
	}

	void write_double(uint8_t* data, double inValue, Endian desired)
	{
		uint8_t number_data[8];

		if (wants_byte_flip(desired)) 
		{
			std::memmove(number_data, &inValue, 8);
			data[0] = number_data[7];
			data[1] = number_data[6];
			data[2] = number_data[5];
			data[3] = number_data[4];
			data[4] = number_data[3];
			data[5] = number_data[2];
			data[6] = number_data[1];
			data[7] = number_data[0];
		}
		else 
		{
			std::memmove(data, &inValue, 8);
		}
	}

	void write_int64(uint8_t* outData, int64_t inValue, Endian desired)
	{
		if (wants_byte_flip(desired))
		{
			outData[0] = inValue >> 56;
			outData[1] = (inValue >> 48) & 0xFF;
			outData[2] = (inValue >> 40) & 0xFF;
			outData[3] = (inValue >> 32) & 0xFF;
			outData[4] = (inValue >> 24) & 0xFF;
			outData[5] = (inValue >> 16) & 0xFF;
			outData[6] = (inValue >> 8) & 0xFF;
			outData[7] = inValue & 0xFF;
		}
		else
		{
			outData[0] = inValue & 0xFF;
			outData[1] = (inValue >> 8) & 0xFF;
			outData[2] = (inValue >> 16) & 0xFF;
			outData[3] = (inValue >> 24) & 0xFF;
			outData[2] = (inValue >> 32) & 0xFF;
			outData[2] = (inValue >> 40) & 0xFF;
			outData[2] = (inValue >> 48) & 0xFF;
			outData[2] = inValue >> 56;
		}
	}

	void write_int32(uint8_t* outData, int32_t inValue, Endian desired)
	{
		if (wants_byte_flip(desired)) 
		{
			outData[0] = inValue >> 24;
			outData[1] = (inValue >> 16) & 0xFF;
			outData[2] = (inValue >> 8) & 0xFF;
			outData[3] = inValue & 0xFF;
		}
		else 
		{
			outData[0] = inValue & 0xFF;
			outData[1] = (inValue >> 8) & 0xFF;
			outData[2] = (inValue >> 16) & 0xFF;
			outData[3] = inValue >> 24;
		}
	}

	void write_int24(uint8_t* outData, int32_t inValue, Endian desired)
	{
		if (wants_byte_flip(desired)) 
		{
			outData[0] = inValue >> 16;
			outData[1] = (inValue >> 8) & 0xFF;
			outData[2] = inValue & 0xFF;
		}
		else 
		{
			std::memmove(outData, &inValue, 3);
		}
	}

	void write_int16(uint8_t* outData, int16_t value, Endian desired)
	{
		uint8_t int_data[2];

		if (wants_byte_flip(desired)) 
		{
			std::memmove(int_data, &value, 2);
			outData[0] = int_data[1];
			outData[1] = int_data[0];
		}
		else 
		{
			std::memmove(outData, &value, 2);
		}
	}

}; // SerializationUtils

// -- BinaryWriter -----
BinaryWriter::BinaryWriter(std::vector<uint8_t>& inBuffer) : m_buffer(inBuffer) 
{
}

void BinaryWriter::appendByte(uint8_t value)
{
	m_buffer.push_back(value);
}

void BinaryWriter::appendBytes(const uint8_t* byteArray, size_t byteCount)
{
	if (byteCount > 0)
		m_buffer.insert(m_buffer.end(), byteArray, byteArray + byteCount);
}

void to_binary(BinaryWriter& writer, bool inValue)
{
	writer.appendByte(inValue ? 1 : 0);
}

void to_binary(BinaryWriter& writer, uint8_t inValue)
{
	writer.appendBytes((const uint8_t*)&inValue, 1);
}

void to_binary(BinaryWriter& writer, uint16_t inValue)
{
	std::array<uint8_t, sizeof(uint16_t)> outValue;
	Serialization::write_int16(outValue.data(), *(int16_t*)&inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, uint32_t inValue)
{
	std::array<uint8_t, sizeof(int32_t)> outValue;
	Serialization::write_int32(outValue.data(), *(int32_t*)&inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, uint64_t inValue)
{
	std::array<uint8_t, sizeof(int64_t)> outValue;
	Serialization::write_int64(outValue.data(), *(int64_t*)&inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, int8_t inValue)
{
	writer.appendBytes((const uint8_t* )&inValue, 1);
}

void to_binary(BinaryWriter& writer, int16_t inValue)
{
	std::array<uint8_t, sizeof(int16_t)> outValue;
	Serialization::write_int16(outValue.data(), inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, int32_t inValue)
{
	std::array<uint8_t, sizeof(int32_t)> outValue;
	Serialization::write_int32(outValue.data(), inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, int64_t inValue)
{
	std::array<uint8_t, sizeof(int64_t)> outValue;
	Serialization::write_int64(outValue.data(), inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, float inValue)
{
	std::array<uint8_t, sizeof(float)> outValue;
	Serialization::write_float(outValue.data(), inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, double inValue)
{
	std::array<uint8_t, sizeof(double)> outValue;
	Serialization::write_double(outValue.data(), inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, const std::string& inString)
{
	const size_t stringLength = inString.length();

	to_binary(writer, (int32_t)stringLength);
	writer.appendBytes((const uint8_t*)inString.c_str(), stringLength);
}

// -- BinaryReader -----
BinaryReader::BinaryReader(const uint8_t* buffer, size_t bufferSize)
	: m_buffer(buffer), m_bufferSize(bufferSize), m_bytesRead(0)
{
}

uint8_t BinaryReader::readByte()
{
	if (m_bytesRead >= m_bufferSize)
	{
		throw std::out_of_range("BinaryReader::readByte() - No more bytes to read");
	}

	return m_buffer[m_bytesRead++];
}

void BinaryReader::readBytes(uint8_t* outBuffer, size_t byteCount)
{
	if (m_bytesRead + byteCount > m_bufferSize)
	{
		throw std::out_of_range("BinaryReader::readBytes() - Not enough bytes to read");
	}

	std::memmove(outBuffer, m_buffer + m_bytesRead, byteCount);
	m_bytesRead += byteCount;
}

uint8_t* BinaryReader::readBytesNoCopy(size_t byteCount)
{
	if (m_bytesRead + byteCount > m_bufferSize)
	{
		throw std::out_of_range("BinaryReader::readBytesNoCopy() - Not enough bytes to read");
	}

	uint8_t* result = const_cast<uint8_t*>(m_buffer + m_bytesRead);
	m_bytesRead += byteCount;

	return result;
}

void from_binary(BinaryReader& reader, bool& outValue)
{
	outValue= reader.readByte() != 0;
}

void from_binary(BinaryReader& reader, int16_t& outValue)
{
	std::array<uint8_t, sizeof(int16_t)> value;
	reader.readBytes(value.data(), sizeof(int16_t));
	outValue= Serialization::read_int16(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, int32_t& outValue)
{
	std::array<uint8_t, sizeof(int32_t)> value;
	reader.readBytes(value.data(), sizeof(int32_t));
	outValue = Serialization::read_int32(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, float& outValue)
{
	std::array<uint8_t, sizeof(float)> value;
	reader.readBytes(value.data(), sizeof(float));
	outValue = Serialization::read_float(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, double& outValue)
{
	std::array<uint8_t, sizeof(double)> value;
	reader.readBytes(value.data(), sizeof(double));
	outValue = Serialization::read_double(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, std::string& outString)
{
	int32_t stringLength;
	from_binary(reader, stringLength);

	outString= std::string((const char*)reader.readBytesNoCopy(stringLength), stringLength);
}