#pragma once

#include <RmlUi/Core/Platform.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>

namespace Rml {

class InputType;

/**
	A form control extension for the numeric input element. All functionality is handled through an input type interface.

	@author Brendan Walker
 */

class ElementFormControlNumericInput : public ElementFormControlInput
{
public:
	RMLUI_RTTI_DefineWithParent(ElementFormControlNumericInput, ElementFormControlInput)

	/// Constructs a new ElementFormControlInput. This should not be called directly; use the
	/// Factory instead.
	/// @param[in] tag The tag the element was declared as in RML.
	ElementFormControlNumericInput(const String& tag);
	virtual ~ElementFormControlNumericInput();

protected:
	/// Checks for necessary functional changes in the control as a result of changed attributes.
	/// @param[in] changed_attributes The list of changed attributes.
	void OnAttributeChange(const ElementAttributes& changed_attributes) override;

private:
	UniquePtr<InputType> input_type;
	String type_name;
};

} // namespace Rml