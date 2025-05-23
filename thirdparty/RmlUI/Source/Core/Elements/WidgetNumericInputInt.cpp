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

void WidgetNumericInputInt::OnValueDecrement()
{
	const int old_value = GetIntValue();
	const int desired_new_value = old_value - step;
	const int new_value = desired_new_value > min_value ? desired_new_value : min_value;

	if (new_value < old_value)
	{
		SetValue(std::to_string(new_value));
		LineBreak();
	}
}

void WidgetNumericInputInt::OnValueIncrement()
{
	const int old_value = GetIntValue();
	const int desired_new_value = old_value + step;
	const int new_value = (desired_new_value < max_value) ? desired_new_value : max_value;

	if (new_value > old_value)
	{
		SetValue(std::to_string(new_value));
		LineBreak();
	}
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetNumericInputInt::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-';
}

int WidgetNumericInputInt::GetIntValue() const
{
	try
	{
		const Rml::String& stringValue = GetTextElementConst()->GetText();

		return std::stoi(stringValue.c_str());
	}
	catch (std::exception e)
	{
		return 0.f;
	}
}

// Strips all non-numeric characters from the string.
String WidgetNumericInputInt::SanitiseValue(const String& inValue)
{
	int parsedInt;

	try
	{
		int desired_int_value= std::stoi(inValue);
		parsedInt = desired_int_value < max_value ? desired_int_value : max_value;
		parsedInt = parsedInt > min_value ? parsedInt : min_value;
	}
	catch (std::exception e)
	{
		parsedInt= 0;
	}

	return std::to_string(parsedInt);
}

} // namespace Rml
