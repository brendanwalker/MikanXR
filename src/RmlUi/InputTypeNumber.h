#pragma once

#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Types.h>

namespace Rml {

class ElementFormControlNumericInput;
class WidgetNumericInput;

class InputTypeNumber
{
public:
	enum NumberType
	{
		FLOAT,
		INT
	};

	InputTypeNumber(ElementFormControlNumericInput* element, NumberType numberType = FLOAT);
	virtual ~InputTypeNumber();

	/// Returns a string representation of the current value of the form control.
	/// @return The value of the form control.
	virtual String GetValue() const;
	/// Returns if this value should be submitted with the form.
	/// @return True if the form control is to be submitted, false otherwise.
	virtual bool IsSubmitted();

	/// Called every update from the host element.
	virtual void OnUpdate();

	/// Called every render from the host element.
	virtual void OnRender();

	/// Called when the parent element's size changes.
	virtual void OnResize();

	/// Called when the parent element is layed out.
	virtual void OnLayout();

	/// Checks for necessary functional changes in the control as a result of changed attributes.
	/// @param[in] changed_attributes The list of changed attributes.
	/// @return True if no layout is required, false if the layout needs to be dirtied.
	virtual bool OnAttributeChange(const ElementAttributes& changed_attributes);

	/// Called when properties on the control are changed.
	/// @param[in] changed_properties The properties changed on the element.
	virtual void OnPropertyChange(const PropertyIdSet& changed_properties);

	/// Called when the element is added into a hierarchy.
	virtual void OnChildAdd();
	/// Called when the element is removed from a hierarchy.
	virtual void OnChildRemove();

	/// Checks for necessary functional changes in the control as a result of the event.
	/// @param[in] event The event to process.
	virtual void ProcessDefaultAction(Event& event);

	/// Sizes the dimensions to the element's inherent size.
	/// @return True.
	virtual bool GetIntrinsicDimensions(Vector2f& dimensions, float& ratio);

protected:
	ElementFormControlNumericInput* element;

	NumberType m_numberType;
	int size;

	WidgetNumericInput* widget;
};

} // namespace Rml
