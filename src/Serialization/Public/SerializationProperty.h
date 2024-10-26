#pragma once

#include "SerializationExport.h"

#if defined(ENABLE_SERIALIZATION_REFLECTION) || defined(KODGEN_PARSING)
#include <Refureku/Properties/PropertySettings.h>
#include <Refureku/TypeInfo/Archetypes/EnumValue.h>

#ifndef KODGEN_PARSING
#include "SerializationProperty.rfkh.h"
#endif // !KODGEN_PARSING

namespace rfk
{
	class Enum;
	class EnumValue;
}

namespace Serialization NAMESPACE()
{
	class SERIALIZATION_API CLASS(rfk::PropertySettings(rfk::EEntityKind::EnumValue, false, true)) 
		EnumStringValue : public rfk::Property
	{
	public:
		EnumStringValue() noexcept = default;
		EnumStringValue(char const* message) noexcept;
		virtual ~EnumStringValue();

		char const* getValue() const { return m_stringValue; }

		#ifndef KODGEN_PARSING
		Serialization_EnumStringValue_GENERATED
		#endif // !KODGEN_PARSING

	private:
		char* m_stringValue= nullptr;
	};

	SERIALIZATION_API rfk::EnumValue const* findEnumValueByString(rfk::Enum const& enumType, char const* stringValue);
	SERIALIZATION_API char const* getEnumStringValue(rfk::EnumValue const& enumValue);
}

#ifndef KODGEN_PARSING
File_SerializationProperty_GENERATED
#endif // !KODGEN_PARSING

#endif // ENABLE_SERIALIZATION_REFLECTION || KODGEN_PARSING