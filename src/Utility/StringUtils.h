#pragma once

#include "stdlib.h" // size_t
#include <string>
#include <vector>

//-- utility methods -----
namespace StringUtils
{
	template <typename t_enum_class>
	static t_enum_class FindEnumValue(const std::string& stringValue, const std::string k_deviceTypeStrings[])
	{
		for (int enumIntValue = 0; enumIntValue < (int)t_enum_class::COUNT; ++enumIntValue)
		{
			if (k_deviceTypeStrings[enumIntValue] == stringValue)
			{
				return (t_enum_class)enumIntValue;
			}
		}

		return t_enum_class::INVALID;
	}

	bool convertWcsToMbs(const wchar_t* wc_string, char* out_mb_string, const size_t mb_buffer_size);
	bool convertMbsToWcs(const char* mb_string, wchar_t* out_wc_string, const size_t wc_buffer_size);

	std::wstring convertUTF8StringToWString(const std::string& str);
	std::string convertWStringToUTF8String(const std::wstring& wstr);

	/// Formats a string into the given target buffer
	/// \param buffer The target buffer to write in to
	/// \param buffer_size The max number of bytes that can be written to the buffer
	/// \param format The formatting string that will be written to the buffer
	/// \return The number of characters successfully written
	int formatString(char* buffer, size_t buffer_size, const char* format, ...);

	// Split a string on a single character seperator
	std::vector<std::string> splitString(const std::string& s, char seperator);

	// Join a set of strings together with the given delimeter
	std::string joinString(const std::vector<std::string>& elems, char delim);

	/// Formats a wide character string into the given target buffer
	/// \param buffer The target buffer to write in to
	/// \param buffer_size The max number of bytes that can be written to the buffer
	/// \param format The formatting string that will be written to the buffer
	/// \return The number of characters successfully written
	int formatWString(wchar_t* buffer, size_t buffer_size, const wchar_t* format, ...);
};