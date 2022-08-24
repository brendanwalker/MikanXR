#pragma once

#include "stdint.h"

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
