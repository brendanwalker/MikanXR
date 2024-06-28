#pragma once

#include <RmlUi/Core/Platform.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>

namespace Rml {

class InputTypeNumber;

/**
	A form control addition for a numeric input element.
	Copied and adapted from ElementFormControlInput since that class can't be extended due to private members.
	All functionality is handled through an input type interface.

	@author Brendan Walker
 */

class ElementFormControlNumericInput : public ElementFormControl
{
public:
	RMLUI_RTTI_DefineWithParent(ElementFormControlNumericInput, ElementFormControl)

	/// Constructs a new ElementFormControlInput. This should not be called directly; use the
	/// Factory instead.
	/// @param[in] tag The tag the element was declared as in RML.
	ElementFormControlNumericInput(const String& tag);
	virtual ~ElementFormControlNumericInput();

	/// Returns a string representation of the current value of the form control.
	/// @return The value of the form control.
	String GetValue() const override;
	/// Sets the current value of the form control.
	/// @param value[in] The new value of the form control.
	void SetValue(const String& value) override;
	/// Returns if this value's type should be submitted with the form.
	/// @return True if the form control is to be submitted, false otherwise.
	bool IsSubmitted() override;

protected:
	/// Updates the element's underlying type.
	void OnUpdate() override;
	/// Renders the element's underlying type.
	void OnRender() override;
	/// Calls the element's underlying type.
	void OnResize() override;
	/// Calls the element's underlying type.
	void OnLayout() override;

	/// Checks for necessary functional changes in the control as a result of changed attributes.
	/// @param[in] changed_attributes The list of changed attributes.
	void OnAttributeChange(const ElementAttributes& changed_attributes) override;
	/// Called when properties on the control are changed.
	/// @param[in] changed_properties The properties changed on the element.
	void OnPropertyChange(const PropertyIdSet& changed_properties) override;

	/// If we are the added element, this will pass the call onto our type handler.
	/// @param[in] child The new member of the hierarchy.
	void OnChildAdd(Element* child) override;
	/// If we are the removed element, this will pass the call onto our type handler.
	/// @param[in] child The member of the hierarchy that was just removed.
	void OnChildRemove(Element* child) override;

	/// Checks for necessary functional changes in the control as a result of the event.
	/// @param[in] event The event to process.
	void ProcessDefaultAction(Event& event) override;

	/// Sizes the dimensions to the element's inherent size.
	/// @return True.
	bool GetIntrinsicDimensions(Vector2f& dimensions, float& ratio) override;

private:
	UniquePtr<InputTypeNumber> type;
	String type_name;
};

} // namespace Rml