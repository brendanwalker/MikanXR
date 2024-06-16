#include "SerializationUtils.h"
#include <cstring>

namespace SerializationUtils
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

void to_binary(BinaryWriter& writer, int16_t inValue)
{
	std::array<uint8_t, sizeof(int16_t)> outValue;
	SerializationUtils::write_int16(outValue.data(), outValue.size(), SerializationUtils::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, int32_t inValue)
{
	std::array<uint8_t, sizeof(int32_t)> outValue;
	SerializationUtils::write_int32(outValue.data(), outValue.size(), SerializationUtils::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, float inValue)
{
	std::array<uint8_t, sizeof(float)> outValue;
	SerializationUtils::write_float(outValue.data(), outValue.size(), SerializationUtils::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, double inValue)
{
	std::array<uint8_t, sizeof(double)> outValue;
	SerializationUtils::write_double(outValue.data(), outValue.size(), SerializationUtils::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, const std::string& inString)
{
	const size_t stringLength = inString.length();

	to_binary(writer, (int32_t)stringLength);
	writer.appendBytes((const uint8_t*)inString.c_str(), stringLength);
}
