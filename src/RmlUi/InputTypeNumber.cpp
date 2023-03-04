#include "InputTypeNumber.h"
#include "WidgetNumericInputFloat.h"
#include "WidgetNumericInputInt.h"
#include "ElementFormControlNumericInput.h"
#include <RmlUi/Core/ElementUtilities.h>
//#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/PropertyIdSet.h>

namespace Rml {

InputTypeNumber::InputTypeNumber(ElementFormControlNumericInput* in_element, NumberType numberType) 
	: element(in_element)
	, m_numberType(numberType)
{
	if (m_numberType == NumberType::FLOAT)
	{
		WidgetNumericInputFloat* floatWidget = new WidgetNumericInputFloat(element);
		floatWidget->SetPrecision(element->GetAttribute< int >("precision", 2));

		widget = floatWidget;
	}
	else
	{
		widget = new WidgetNumericInputInt(element);
	}

	widget->Initialize();
	widget->SetMaxLength(element->GetAttribute< int >("maxlength", -1));
	widget->SetValue(element->GetAttribute< String >("value", ""));

	size = element->GetAttribute< int >("size", 20);
}

InputTypeNumber::~InputTypeNumber()
{
	delete widget;
}

// Returns a string representation of the current value of the form control.
String InputTypeNumber::GetValue() const
{
	return element->GetAttribute< String >("value", "");
}

// Returns if this value should be submitted with the form.
bool InputTypeNumber::IsSubmitted()
{
	return true;
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
		WidgetNumericInputFloat* floatWidget= (WidgetNumericInputFloat *)widget;

		auto it_step = changed_attributes.find("step");
		if (it_step != changed_attributes.end())
			floatWidget->SetStep(it_step->second.Get<float>(1.0f));

		auto it_min = changed_attributes.find("min");
		if (it_min != changed_attributes.end())
			floatWidget->SetMinValue(it_min->second.Get<float>(0.0f));

		auto it_max = changed_attributes.find("max");
		if (it_max != changed_attributes.end())
			floatWidget->SetMaxValue(it_max->second.Get<float>(100.f));

		auto it = changed_attributes.find("precision");
		if (it != changed_attributes.end())
		{
			int newPrecision= it->second.Get(-1);

			if (newPrecision != -1)
			{
				floatWidget->SetPrecision(newPrecision);
			}
		}
	}
	else if (m_numberType == NumberType::INT)
	{
		WidgetNumericInputInt* intWidget = (WidgetNumericInputInt*)widget;

		auto it_step = changed_attributes.find("step");
		if (it_step != changed_attributes.end())
			intWidget->SetStep(it_step->second.Get<int>(1));

		auto it_min = changed_attributes.find("min");
		if (it_min != changed_attributes.end())
			intWidget->SetMinValue(it_min->second.Get<int>(0));

		auto it_max = changed_attributes.find("max");
		if (it_max != changed_attributes.end())
			intWidget->SetMaxValue(it_max->second.Get<int>(100));
	}

	// Check if the value has been changed.
	it = changed_attributes.find("value");
	if (it != changed_attributes.end())
	{
		const String rawValue= it->second.Get<String>();
		const String sanitizedValue= widget->SanitiseValue(rawValue);

		widget->SetValue(sanitizedValue);
	}

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

// Called when the element is added into a hierarchy.
void InputTypeNumber::OnChildAdd()
{
}

// Called when the element is removed from a hierarchy.
void InputTypeNumber::OnChildRemove()
{
}

} // namespace Rml
