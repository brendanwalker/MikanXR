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

	template <typename T>
	T read_bytes(const uint8_t* inData, Endian desired)
	{
		const size_t byteCount = sizeof(T);
		T result= T();

		if (wants_byte_flip(desired))
		{
			uint8_t* valueByte = (uint8_t*)&result;

			for (int64_t i = (int64_t)byteCount - 1; i >= 0; i--)
			{
				*valueByte= inData[i];
				valueByte++;
			}
		}
		else
		{
			std::memmove(&result, inData, byteCount);
		}

		return result;
	}

	int16_t read_int16(const uint8_t* inData, Endian desired)
	{
		return read_bytes<int16_t>(inData, desired);
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
		return read_bytes<int32_t>(inData, desired);
	}

	int64_t read_int64(const uint8_t* inData, Endian desired)
	{
		return read_bytes<int64_t>(inData, desired);
	}

	float read_float(const uint8_t* inData, Endian desired)
	{
		return read_bytes<float>(inData, desired);
	}

	double read_double(const uint8_t* inData, Endian desired)
	{
		return read_bytes<double>(inData, desired);
	}

	template <typename T>
	void write_bytes(uint8_t* outData, const T& value, Endian desired)
	{
		const size_t byteCount = sizeof(T);

		if (wants_byte_flip(desired))
		{
			const uint8_t* valueByte = (const uint8_t*)&value;

			for (int64_t i = (int64_t)byteCount - 1; i >= 0; i--)
			{
				outData[i] = *valueByte;
				valueByte++;
			}
		}
		else
		{
			std::memmove(outData, &value, byteCount);
		}
	}

	void write_float(uint8_t* data, float inValue, Endian desired)
	{
		write_bytes<float>(data, inValue, desired);
	}

	void write_double(uint8_t* data, double inValue, Endian desired)
	{
		write_bytes<double>(data, inValue, desired);
	}

	void write_int64(uint8_t* data, int64_t inValue, Endian desired)
	{
		write_bytes<int64_t>(data, inValue, desired);
	}

	void write_int32(uint8_t* data, int32_t inValue, Endian desired)
	{
		write_bytes<int32_t>(data, inValue, desired);
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

	void write_int16(uint8_t* data, int16_t inValue, Endian desired)
	{
		write_bytes<int16_t>(data, inValue, desired);
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
	std::array<uint8_t, sizeof(uint32_t)> outValue;
	Serialization::write_int32(outValue.data(), *(int32_t*)&inValue, Serialization::Endian::Little);

	writer.appendBytes(outValue);
}

void to_binary(BinaryWriter& writer, uint64_t inValue)
{
	std::array<uint8_t, sizeof(uint64_t)> outValue;
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

void from_binary(BinaryReader& reader, uint8_t& outValue)
{
	outValue= reader.readByte();
}

void from_binary(BinaryReader& reader, uint16_t& outValue)
{
	std::array<uint8_t, sizeof(uint16_t)> value;
	reader.readBytes(value.data(), sizeof(uint16_t));
	outValue = Serialization::read_int16(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, uint32_t& outValue)
{
	std::array<uint8_t, sizeof(uint32_t)> value;
	reader.readBytes(value.data(), sizeof(uint32_t));
	outValue = Serialization::read_int32(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, uint64_t& outValue)
{
	std::array<uint8_t, sizeof(uint64_t)> value;
	reader.readBytes(value.data(), sizeof(uint64_t));
	outValue = Serialization::read_int64(value.data(), Serialization::Endian::Little);
}

void from_binary(BinaryReader& reader, int8_t& outValue)
{
	reader.readBytes((uint8_t*)&outValue, 1);
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

void from_binary(BinaryReader& reader, int64_t& outValue)
{
	std::array<uint8_t, sizeof(int64_t)> value;
	reader.readBytes(value.data(), sizeof(int64_t));
	outValue = Serialization::read_int64(value.data(), Serialization::Endian::Little);
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