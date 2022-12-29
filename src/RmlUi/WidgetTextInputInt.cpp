#include <RmlUi/Core/ElementText.h>
#include "WidgetTextInputInt.h"

namespace Rml {

WidgetTextInputInt::WidgetTextInputInt(ElementFormControl* parent) : WidgetTextInput(parent)
{
}

WidgetTextInputInt::~WidgetTextInputInt()
{
}

// Sets the value of the text field. The value will be stripped of all non numerical characters.
void WidgetTextInputInt::SetValue(const String& value)
{
	String new_value = value;
	SanitiseValue(new_value);

	WidgetTextInput::SetValue(new_value);
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetTextInputInt::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-' || character == '+';
}

// Called when the user pressed enter.
void WidgetTextInputInt::LineBreak()
{
	DispatchChangeEvent(true);
}

// Strips all \n and \r characters from the string.
void WidgetTextInputInt::SanitiseValue(String& value)
{
	try
	{
		int parsedInt = std::stoi(value);
		value = std::to_string(parsedInt);
	}
	catch (std::exception e)
	{
		value = "0";
	}
}

} // namespace Rml
