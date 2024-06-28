#pragma once

#include "WidgetNumericInput.h"

namespace Rml {

/**
	A specialization of the text input widget for integer input fields.

	@author Brendan Walker
 */

class WidgetNumericInputInt : public WidgetNumericInput
{
public:
	WidgetNumericInputInt(ElementFormControl* parent);
	virtual ~WidgetNumericInputInt();

	/// Sets the minimum value of the slider.
	void SetMinValue(int min_value);
	/// Sets the maximum value of the slider.
	void SetMaxValue(int max_value);
	/// Sets the slider's value increment.
	void SetStep(int step);

	void OnValueDecrement() override;
	void OnValueIncrement() override;

	/// Strips all non-numeric characters from the string.
	String SanitiseValue(const String& value);

protected:
	int GetIntValue() const;

	/// Returns true if the current numeric text content is valid numeric value
	/// @param[in] character The character to validate.
	/// @return True if the content is valid, false if not.
	bool IsNumericContentValid(const String& content) override;
	/// Returns true if the given character is permitted in the input field, false if not.
	/// @param[in] character The character to validate.
	/// @return True if the character is allowed, false if not.
	bool IsCharacterValid(char character) override;

	int min_value= 0;
	int max_value= 100;
	int step= 1;
};

} // namespace Rml
