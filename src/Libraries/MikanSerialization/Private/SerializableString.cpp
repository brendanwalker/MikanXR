#include "SerializableString.h"
#include "SerializableString.rfks.h"

#include <string>

namespace Serialization
{
struct StringData
{
	std::string value;

	StringData()= default;
	StringData(const char* cstring)
		: value{cstring ? cstring : ""}
	{
	}
};

String::String()
	: m_pimpl{new StringData()}
{
}
String::String(const char* cstring)
	: m_pimpl{new StringData(cstring)}
{
}
String::String(const String& other)
	: m_pimpl{new StringData()}
{
	m_pimpl->value= other.m_pimpl->value;
}
String::String(String&& other) noexcept
	: m_pimpl{new StringData()}
{
	m_pimpl->value= std::move(other.m_pimpl->value);
}
String::~String() { delete m_pimpl; }

String& String::operator=(const char* other)
{
	m_pimpl->value= other ? other : "";
	return *this;
}

String& String::operator=(const String& other)
{
	m_pimpl->value= other.m_pimpl->value;
	return *this;
}

void String::setUtf8Value(const char* str) { m_pimpl->value= str ? str : ""; }

const char* String::getUtf8Value() const { return m_pimpl->value.c_str(); }

bool String::isEmpty() const { return m_pimpl->value.empty(); }

bool String::operator==(const char* other) const { return m_pimpl->value == (other ? other : ""); }

bool String::operator!=(const char* other) const { return !(*this == other); }

bool String::operator==(String const& other) const { return m_pimpl->value == other.m_pimpl->value; }

bool String::operator!=(String const& other) const { return m_pimpl->value != other.m_pimpl->value; }
} // namespace Serialization