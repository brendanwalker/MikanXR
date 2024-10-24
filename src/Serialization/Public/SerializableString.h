#pragma once


#include <string>

#ifndef KODGEN_PARSING
#include "SerializableString.rfkh.h"
#endif

namespace Serialization NAMESPACE()
{
	class CLASS() String
	{
	public:
		String() = default;
		String(const char* cstring) : m_string{cstring} {}
		String(std::string const& string) : m_string{string} {}
		String(std::string&& string) : m_string{std::move(string)} {}

		void setValue(std::string const& string)
		{
			m_string = string;
		}
		const std::string& getValue() const
		{
			return m_string;
		}

		String& operator=(std::string const& string)
		{
			m_string = string;
			return *this;
		}

		bool operator==(std::string const& other) const
		{
			return m_string == other;
		}

		bool operator!=(std::string const& other) const
		{
			return m_string != other;
		}

		bool operator==(String const& other) const
		{
			return m_string == other.m_string;
		}

		bool operator!=(String const& other) const
		{
			return m_string != other.m_string;
		}

		#ifndef KODGEN_PARSING
		Serialization_String_GENERATED
		#endif

	private:
		std::string m_string;
	};
};

#ifndef KODGEN_PARSING
File_SerializableString_GENERATED
#endif