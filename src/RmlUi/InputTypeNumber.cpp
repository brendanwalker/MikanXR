#include "InputTypeNumber.h"
#include "WidgetTextInputFloat.h"
#include "WidgetTextInputInt.h"
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/PropertyIdSet.h>

namespace Rml {

InputTypeNumber::InputTypeNumber(ElementFormControlInput* element, NumberType numberType) 
	: InputType(element)
	, m_numberType(numberType)
{
	if (m_numberType == NumberType::FLOAT)
	{
		WidgetTextInputFloat* floatWidget = new WidgetTextInputFloat(element);
		floatWidget->SetPrecision(element->GetAttribute< int >("precision", 2));

		widget = floatWidget;
	}
	else
	{
		widget = new WidgetTextInputInt(element);
	}

	widget->SetMaxLength(element->GetAttribute< int >("maxlength", -1));
	widget->SetValue(element->GetAttribute< String >("value", ""));

	size = element->GetAttribute< int >("size", 20);
}

InputTypeNumber::~InputTypeNumber()
{
	delete widget;
}

// Called every update from the host element.
void InputTypeNumber::OnUpdate()
{
	widget->OnUpdate();
}

// Called every render from the host element.
void InputTypeNumber::OnRender()
{
	widget->OnRender();
}

void InputTypeNumber::OnResize()
{
	widget->OnResize();
}

void InputTypeNumber::OnLayout()
{
	widget->OnLayout();
}

// Checks for necessary functional changes in the control as a result of changed attributes.
bool InputTypeNumber::OnAttributeChange(const ElementAttributes& changed_attributes)
{
	bool dirty_layout = false;

	// Check if maxlength has been defined.
	auto it = changed_attributes.find("maxlength");
	if (it != changed_attributes.end())
		widget->SetMaxLength(it->second.Get(-1));

	// Check if size has been defined.
	it = changed_attributes.find("size");
	if (it != changed_attributes.end())
	{
		size = it->second.Get(20);
		dirty_layout = true;
	}

	if (m_numberType == NumberType::FLOAT)
	{
		auto it = changed_attributes.find("precision");
		if (it != changed_attributes.end())
		{
			int newPrecision= it->second.Get(-1);

			if (newPrecision != -1)
			{
				((WidgetTextInputFloat*)widget)->SetPrecision(newPrecision);
			}
		}
	}

	// Check if the value has been changed.
	it = changed_attributes.find("value");
	if (it != changed_attributes.end())
		widget->SetValue(it->second.Get<String>());

	return !dirty_layout;
}

// Called when properties on the control are changed.
void InputTypeNumber::OnPropertyChange(const PropertyIdSet& changed_properties)
{
	if (changed_properties.Contains(PropertyId::Color) ||
		changed_properties.Contains(PropertyId::BackgroundColor))
		widget->UpdateSelectionColours();

	if (changed_properties.Contains(PropertyId::CaretColor))
		widget->GenerateCursor();
}

// Checks for necessary functional changes in the control as a result of the event.
void InputTypeNumber::ProcessDefaultAction(Event& RMLUI_UNUSED_PARAMETER(event))
{
	RMLUI_UNUSED(event);
}

// Sizes the dimensions to the element's inherent size.
bool InputTypeNumber::GetIntrinsicDimensions(Vector2f& dimensions, float& /*ratio*/)
{
	dimensions.x = (float) (size * ElementUtilities::GetStringWidth(element, "0"));
	dimensions.y = element->GetLineHeight() + 2.0f;

	return true;
}

} // namespace Rml
