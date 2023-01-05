#include <RmlUi/Core/ElementText.h>
#include "WidgetNumericInputInt.h"

namespace Rml {

WidgetNumericInputInt::WidgetNumericInputInt(ElementFormControl* parent) : WidgetNumericInput(parent)
{
}

WidgetNumericInputInt::~WidgetNumericInputInt()
{
}

bool WidgetNumericInputInt::IsNumericContentValid(const String& content)
{
	try
	{
		int unusedInt= std::stoi(content.c_str());
		return true;
	}
	catch (std::exception e)
	{
		return false;
	}
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetNumericInputInt::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-' || character == '+';
}

// Strips all non-numeric characters from the string.
String WidgetNumericInputInt::SanitiseValue(const String& inValue)
{
	int parsedInt;

	try
	{
		parsedInt = std::stoi(inValue);
	}
	catch (std::exception e)
	{
		parsedInt= 0;
	}

	return std::to_string(parsedInt);
}

} // namespace Rml
