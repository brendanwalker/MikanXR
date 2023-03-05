#include <RmlUi/Core/ElementText.h>
#include "WidgetNumericInputInt.h"
#include "MathUtility.h"

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

// Sets the minimum value of the slider.
void WidgetNumericInputInt::SetMinValue(int _min_value)
{
	min_value = _min_value;
}

// Sets the maximum value of the slider.
void WidgetNumericInputInt::SetMaxValue(int _max_value)
{
	max_value = _max_value;
}

// Sets the slider's value increment.
void WidgetNumericInputInt::SetStep(int _step)
{
	// Can't have a zero step!
	if (_step == 0)
		return;

	step = _step;
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetNumericInputInt::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-';
}

// Strips all non-numeric characters from the string.
String WidgetNumericInputInt::SanitiseValue(const String& inValue)
{
	int parsedInt;

	try
	{
		parsedInt = int_clamp(std::stoi(inValue), min_value, max_value);
	}
	catch (std::exception e)
	{
		parsedInt= 0;
	}

	return std::to_string(parsedInt);
}

} // namespace Rml
