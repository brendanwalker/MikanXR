#pragma once

#include "WidgetTextInput.h"

namespace Rml {

/**
	A specialization of the text input widget for integer input fields.

	@author Brendan Walker
 */

class WidgetTextInputInt : public WidgetTextInput
{
public:
	WidgetTextInputInt(ElementFormControl* parent);
	virtual ~WidgetTextInputInt();

	/// Sets the value of the text field. The value will be stripped of end-lines.
	/// @param value[in] The new value to set on the text field.
	void SetValue(const String& value) override;

protected:
	/// Returns true if the given character is permitted in the input field, false if not.
	/// @param[in] character The character to validate.
	/// @return True if the character is allowed, false if not.
	bool IsCharacterValid(char character) override;
	/// Called when the user pressed enter.
	void LineBreak() override;

	/// Strips all \n and \r characters from the string.
	void SanitiseValue(String& value);
};

} // namespace Rml
