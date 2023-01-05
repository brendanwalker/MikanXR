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

	/// Strips all non-numeric characters from the string.
	String SanitiseValue(const String& value);

protected:
	/// Returns true if the current numeric text content is valid numeric value
	/// @param[in] character The character to validate.
	/// @return True if the content is valid, false if not.
	bool IsNumericContentValid(const String& content) override;
	/// Returns true if the given character is permitted in the input field, false if not.
	/// @param[in] character The character to validate.
	/// @return True if the character is allowed, false if not.
	bool IsCharacterValid(char character) override;
};

} // namespace Rml
