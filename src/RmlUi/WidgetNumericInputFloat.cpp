#include "WidgetNumericInputFloat.h"
#include <RmlUi/Core/ElementText.h>

#include <iomanip>
#include <sstream>

namespace Rml {

WidgetNumericInputFloat::WidgetNumericInputFloat(ElementFormControl* parent) 
	: WidgetNumericInput(parent)
	, m_precision(2)
{
}

WidgetNumericInputFloat::~WidgetNumericInputFloat()
{
}

void WidgetNumericInputFloat::SetPrecision(int precision)
{
	m_precision= precision;
}

bool WidgetNumericInputFloat::IsNumericContentValid(const String& content)
{
	try
	{
		float unusedFloat= std::stof(content.c_str());
		return true;
	}
	catch (std::exception e)
	{
		return false;
	}
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetNumericInputFloat::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-' || character == '+' || character == '.' || character == ',';
}

// Strips all \n and \r characters from the string.
String WidgetNumericInputFloat::SanitiseValue(const String& value)
{
	float parsedFloat;
	
	try
	{
		parsedFloat = std::stof(value);

	}
	catch (std::exception e)
	{
		parsedFloat= 0;
	}

	std::stringstream stream;
	stream << std::fixed << std::setprecision(m_precision) << parsedFloat;
	return stream.str();
}

} // namespace Rml
