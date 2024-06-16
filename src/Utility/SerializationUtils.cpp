#include "SerializationUtils.h"
#include <cstring>

namespace SerializationUtils
{

union endian_checker
{
	long endian_value;
	char byte_array[sizeof(long)];
};
static endian_checker EndianChecker = { 1 };

bool is_little_endian()
{
	return EndianChecker.byte_array[0];
}


bool is_big_endian()
{
	return !EndianChecker.byte_array[0];
}

int16_t read_be16int(const uint8_t* inData)
{
	int16_t result;

	if (is_little_endian()) 
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


int32_t read_be24int(const uint8_t* inData)
{
	int32_t result;

	if (is_little_endian()) 
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

int32_t read_le32int(const uint8_t* data)
{
	int32_t result;

	if (is_little_endian()) {
		result = data[0];
		result += data[1] << 8;
		result += data[2] << 16;
		result += data[3] << 24;
	}
	else 
	{
		result = data[0] << 24;
		result += data[1] << 16;
		result += data[2] << 8;
		result += data[3];
	}
	return result;
}

int32_t read_be32int(const uint8_t* inData)
{
	int32_t result;

	if (is_little_endian()) 
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

double read_be64double(const uint8_t* inData)
{
	double result;
	uint8_t number_data[8];

	if (is_little_endian()) 
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

void write_be64double(uint8_t* data, double inValue)
{
	uint8_t number_data[8];

	if (is_little_endian()) 
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

void write_le32int(uint8_t* outData, int32_t inValue)
{
	if (is_little_endian()) 
	{
		outData[0] = inValue & 0xFF;
		outData[1] = (inValue >> 8) & 0xFF;
		outData[2] = (inValue >> 16) & 0xFF;
		outData[3] = inValue >> 24;
	}
	else 
	{
		outData[0] = inValue >> 24;
		outData[1] = (inValue >> 16) & 0xFF;
		outData[2] = (inValue >> 8) & 0xFF;
		outData[3] = inValue & 0xFF;
	}
}

void write_be32int(uint8_t* outData, int32_t inValue)
{
	if (is_little_endian()) 
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

void write_be24int(uint8_t* outData, int32_t inValue)
{
	if (is_little_endian()) 
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

void write_be16int(uint8_t* outData, int16_t value)
{
	uint8_t int_data[2];

	if (is_little_endian()) 
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
void BinaryWriter::writeInt32(int32_t inValue)
{
	std::array<uint8_t, 4> outValue;
	SerializationUtils::write_le32int(outValue.data(), outValue.size());

	appendBytes(outValue);
}

void BinaryWriter::writeUtf8String(const std::string& inString)
{
	const size_t stringLength = inString.length();

	writeInt32((int32_t)stringLength);
	appendBytes((const uint8_t*)inString.c_str(), stringLength);
}