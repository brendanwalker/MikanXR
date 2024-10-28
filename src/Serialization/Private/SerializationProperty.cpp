#include "SerializationProperty.h"
#include "SerializationProperty.rfks.h"

#include "string.h"

namespace Serialization
{
	// -- EnumStringValue -----
	EnumStringValue::EnumStringValue(char const* value) noexcept
	{
		if (value != nullptr)
		{
			size_t len = strlen(value);
			m_stringValue = new char[len + 1];
			std::memmove(m_stringValue, value, len);
			m_stringValue[len] = '\0';
		}
		else
		{
			m_stringValue = nullptr;
		}
	}

	EnumStringValue::~EnumStringValue()
	{
		if (m_stringValue != nullptr)
		{
			delete[] m_stringValue;
			m_stringValue = nullptr;
		}
	}

	rfk::EnumValue const* findEnumValueByString(rfk::Enum const& enumType, char const* stringValue)
	{
		// First try and find a value with the EnumStringValue property that has a matching string value
		rfk::EnumValue const* enumValue = enumType.getEnumValueByPredicate(
			[](rfk::EnumValue const& ev, void* userdata) {
			const char* desiredValue = (const char*)userdata;
			auto const* property = ev.getProperty<Serialization::EnumStringValue>();
			if (property != nullptr)
			{
				return std::strcmp(property->getValue(), desiredValue) == 0;
			}
			return false;
		}, (void*)stringValue);

		// Fall back to searching by full enum value name
		if (enumValue == nullptr)
		{
			enumValue= enumType.getEnumValueByName(stringValue);
		}

		return enumValue;
	}

	char const* getEnumStringValue(rfk::EnumValue const& enumValue)
	{
		auto const* property = enumValue.getProperty<Serialization::EnumStringValue>();
		if (property != nullptr)
		{
			return property->getValue();
		}

		return enumValue.getName();
	}

	// -- CodeGenTag -----
	CodeGenModule::CodeGenModule(char const* moduleName) noexcept
	{
		assert(moduleName != nullptr && moduleName[0] != '\0');
		size_t len = strlen(moduleName);
		m_moduleName = new char[len + 1];
		std::memmove(m_moduleName, moduleName, len);
		m_moduleName[len] = '\0';
	}

	CodeGenModule::~CodeGenModule()
	{
		delete[] m_moduleName;
	}
}