#include "WidgetTextInputFloat.h"
#include <RmlUi/Core/ElementText.h>

#include <iomanip>
#include <sstream>

namespace Rml {

WidgetTextInputFloat::WidgetTextInputFloat(ElementFormControl* parent) 
	: WidgetTextInput(parent)
	, m_precision(2)
{
}

WidgetTextInputFloat::~WidgetTextInputFloat()
{
}

void WidgetTextInputFloat::SetPrecision(int precision)
{
	m_precision= precision;
}

// Sets the value of the text field. The value will be stripped of all non numerical characters.
void WidgetTextInputFloat::SetValue(const String& value)
{
	String new_value = value;
	SanitiseValue(new_value);

	WidgetTextInput::SetValue(new_value);
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetTextInputFloat::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-' || character == '+' || character == '.' || character == ',';
}

// Called when the user pressed enter.
void WidgetTextInputFloat::LineBreak()
{
	DispatchChangeEvent(true);
}

// Strips all \n and \r characters from the string.
void WidgetTextInputFloat::SanitiseValue(String& value)
{
	try
	{
		float parsedFloat = std::stof(value);

		std::stringstream stream;
		stream << std::fixed << std::setprecision(m_precision) << parsedFloat;
		value = stream.str();
	}
	catch (std::exception e)
	{
		value = "0.0";
	}
}

} // namespace Rml
