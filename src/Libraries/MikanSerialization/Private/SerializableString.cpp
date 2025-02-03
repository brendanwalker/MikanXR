#include "SerializableString.h"
#include "SerializableString.rfks.h"

namespace Serialization
{
	struct StringData
	{
		std::string value;

		StringData() = default;
		StringData(const char* cstring) : value{cstring} {}
		StringData(std::string const& string) : value{string} {}
		StringData(std::string&& string) : value{std::move(string)} {}
	};

	String::String() : m_pimpl{new StringData()} {}
	String::String(const char* cstring) : m_pimpl{new StringData(cstring)} {}
	String::String(std::string const& string) : m_pimpl{new StringData(string)} {}
	String::String(std::string&& string) : m_pimpl{new StringData(std::move(string))} {}
	String::String(const String& other) : m_pimpl{new StringData(other.m_pimpl->value)} {}
	String::String(String&& other) noexcept : m_pimpl{new StringData(other.m_pimpl->value)} {}
	String::~String() { delete m_pimpl; }

	String& String::operator=(const char* other)
	{
		m_pimpl->value= other;
		return *this;
	}

	String& String::operator=(const std::string& other)
	{
		m_pimpl->value= other;
		return *this;
	}

	String& String::operator=(const String& other)
	{ 
		m_pimpl->value= other.m_pimpl->value; 
		return *this; 
	}

	void String::setValue(std::string const& string)
	{
		m_pimpl->value= string;
	}
	const std::string& String::getValue() const
	{
		return m_pimpl->value;
	}

	bool String::operator==(std::string const& other) const
	{
		return m_pimpl->value == other;
	}

	bool String::operator!=(std::string const& other) const
	{
		return m_pimpl->value != other;
	}

	bool String::operator==(String const& other) const
	{
		return m_pimpl->value == other.m_pimpl->value;
	}

	bool String::operator!=(String const& other) const
	{
		return m_pimpl->value != other.m_pimpl->value;
	}
}