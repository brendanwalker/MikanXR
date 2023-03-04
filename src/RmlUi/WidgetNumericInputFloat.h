#pragma once

#include "WidgetNumericInput.h"

namespace Rml {

/**
	A specialization of the text input widget for numeric float input fields.

	@author Brendan Walker
 */

class WidgetNumericInputFloat : public WidgetNumericInput
{
public:
	WidgetNumericInputFloat(ElementFormControl* parent);
	virtual ~WidgetNumericInputFloat();

	/// Sets the minimum value of the slider.
	void SetMinValue(float min_value);
	/// Sets the maximum value of the slider.
	void SetMaxValue(float max_value);
	/// Sets the slider's value increment.
	void SetStep(float step);

	/// Sets the number of decimal places displayed.
	/// @param value[in] See https://en.cppreference.com/w/cpp/io/manip/setprecision.
	void SetPrecision(int precision);

	void OnValueDecrement() override;
	void OnValueIncrement() override;

	/// Strips all non-numeric characters from the string.
	String SanitiseValue(const String& value) override;

protected:
	float GetFloatValue() const;

	/// Returns true if the current numeric text content is valid numeric value
	/// @param[in] character The character to validate.
	/// @return True if the content is valid, false if not.
	bool IsNumericContentValid(const String& content) override;
	/// Returns true if the given character is permitted in the input field, false if not.
	/// @param[in] character The character to validate.
	/// @return True if the character is allowed, false if not.
	bool IsCharacterValid(char character) override;

	int m_precision= 2;
	float min_value = 0.f;
	float max_value = 100.f;
	float step = 1.f;
};

} // namespace Rml
