#pragma once

#include "InputType.h"

namespace Rml {

class WidgetTextInput;

class InputTypeNumber : public InputType
{
public:
	enum NumberType
	{
		FLOAT,
		INT
	};

	InputTypeNumber(ElementFormControlInput* element, NumberType numberType = FLOAT);
	virtual ~InputTypeNumber();

	/// Called every update from the host element.
	void OnUpdate() override;

	/// Called every render from the host element.
	void OnRender() override;

	/// Called when the parent element's size changes.
	void OnResize() override;

	/// Called when the parent element is layed out.
	void OnLayout() override;

	/// Checks for necessary functional changes in the control as a result of changed attributes.
	/// @param[in] changed_attributes The list of changed attributes.
	/// @return True if no layout is required, false if the layout needs to be dirtied.
	bool OnAttributeChange(const ElementAttributes& changed_attributes) override;

	/// Called when properties on the control are changed.
	/// @param[in] changed_properties The properties changed on the element.
	void OnPropertyChange(const PropertyIdSet& changed_properties) override;

	/// Checks for necessary functional changes in the control as a result of the event.
	/// @param[in] event The event to process.
	void ProcessDefaultAction(Event& event) override;

	/// Sizes the dimensions to the element's inherent size.
	/// @return True.
	bool GetIntrinsicDimensions(Vector2f& dimensions, float& ratio) override;

private:
	NumberType m_numberType;
	int size;

	WidgetTextInput* widget;
};

} // namespace Rml
