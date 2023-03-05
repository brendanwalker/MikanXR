#include "WidgetNumericInputFloat.h"
#include "MathUtility.h"
#include <RmlUi/Core/ElementText.h>

#include <iomanip>
#include <sstream>

namespace Rml {

WidgetNumericInputFloat::WidgetNumericInputFloat(ElementFormControl* parent) 
	: WidgetNumericInput(parent)
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

// Sets the minimum value of the slider.
void WidgetNumericInputFloat::SetMinValue(float _min_value)
{
	min_value = _min_value;
}

// Sets the maximum value of the slider.
void WidgetNumericInputFloat::SetMaxValue(float _max_value)
{
	max_value = _max_value;
}

// Sets the slider's value increment.
void WidgetNumericInputFloat::SetStep(float _step)
{
	// Can't have a zero step!
	if (_step == 0)
		return;

	step = _step;
}

// Returns true if the given character is permitted in the input field, false if not.
bool WidgetNumericInputFloat::IsCharacterValid(char character)
{
	return isdigit(character) || character == '-' || character == '.' || character == ',';
}

void WidgetNumericInputFloat::OnValueDecrement()
{
	const float old_value= GetFloatValue();
	const float new_value= fmaxf(old_value - step, min_value);

	if (new_value < old_value)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(m_precision) << new_value;
		std::string newStringValue= stream.str();

		SetValue(newStringValue);
		DispatchChangeEvent();
	}
}

void WidgetNumericInputFloat::OnValueIncrement()
{
	const float old_value = GetFloatValue();
	const float new_value = fminf(old_value + step, max_value);

	if (new_value > old_value)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(m_precision) << new_value;
		std::string newStringValue = stream.str();

		SetValue(newStringValue);
		DispatchChangeEvent();
	}
}

float WidgetNumericInputFloat::GetFloatValue() const
{
	try
	{
		const Rml::String& stringValue= GetTextElementConst()->GetText();

		return std::stof(stringValue.c_str());
	}
	catch (std::exception e)
	{
		return 0.f;
	}
}

// Strips all \n and \r characters from the string.
String WidgetNumericInputFloat::SanitiseValue(const String& value)
{
	float parsedFloat;
	
	try
	{
		parsedFloat = clampf(std::stof(value), min_value, max_value);
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
