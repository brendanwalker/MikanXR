
#include "ElementFormControlNumericInput.h"
#include "InputTypeNumber.h"
#include <RmlUi/Core/Event.h>

namespace Rml {

// Constructs a new ElementFormControlNumericInput.
ElementFormControlNumericInput::ElementFormControlNumericInput(const String& tag) : ElementFormControl(tag)
{
	// OnAttributeChange will be called right after this, possible with a non-default type. Thus,
	// creating the default InputTypeText here may result in it being destroyed in just a few moments.
	// Instead, we create the InputTypeText in OnAttributeChange in the case where the type attribute has not been set.
}

ElementFormControlNumericInput::~ElementFormControlNumericInput()
{}

// Returns a string representation of the current value of the form control.
String ElementFormControlNumericInput::GetValue() const
{
	RMLUI_ASSERT(type);
	return type->GetValue();
}

// Sets the current value of the form control.
void ElementFormControlNumericInput::SetValue(const String& value)
{
	SetAttribute("value", value);
}

// Returns if this value should be submitted with the form.
bool ElementFormControlNumericInput::IsSubmitted()
{
	RMLUI_ASSERT(type);
	return type->IsSubmitted();
}

// Updates the element's underlying type.
void ElementFormControlNumericInput::OnUpdate()
{
	RMLUI_ASSERT(type);
	type->OnUpdate();
}

// Renders the element's underlying type.
void ElementFormControlNumericInput::OnRender()
{
	RMLUI_ASSERT(type);
	type->OnRender();
}

void ElementFormControlNumericInput::OnResize()
{
	RMLUI_ASSERT(type);
	type->OnResize();
}

void ElementFormControlNumericInput::OnLayout()
{
	RMLUI_ASSERT(type);
	type->OnLayout();
}

// Checks for necessary functional changes in the control as a result of changed attributes.
void ElementFormControlNumericInput::OnAttributeChange(const ElementAttributes& changed_attributes)
{
	// Completely bypass ElementFormControlInput::OnAttributeChange
	ElementFormControl::OnAttributeChange(changed_attributes);

	String new_type_name;

	auto it_type = changed_attributes.find("type");
	if (it_type != changed_attributes.end())
	{
		new_type_name = it_type->second.Get<String>("text");
	}

	if (!type || (!new_type_name.empty() && new_type_name != type_name))
	{
		if (new_type_name == "float")
		{
			type = MakeUnique<InputTypeNumber>(this, InputTypeNumber::FLOAT);
		}
		else
		{
			new_type_name = "int";
			type = MakeUnique<InputTypeNumber>(this, InputTypeNumber::INT);
		}

		if (!type_name.empty())
		{
			SetClass(type_name, false);
		}
		SetClass(new_type_name, true);
		type_name = new_type_name;

		DirtyLayout();
	}

	RMLUI_ASSERT(type);

	if (!type->OnAttributeChange(changed_attributes))
	{
		DirtyLayout();
	}
}

// Called when properties on the element are changed.
void ElementFormControlNumericInput::OnPropertyChange(const PropertyIdSet& changed_properties)
{
	ElementFormControl::OnPropertyChange(changed_properties);

	if (type)
		type->OnPropertyChange(changed_properties);
}

// If we are the added element, this will pass the call onto our type handler.
void ElementFormControlNumericInput::OnChildAdd(Element* child)
{
	if (child == this && type)
		type->OnChildAdd();
}

// If we are the removed element, this will pass the call onto our type handler.
void ElementFormControlNumericInput::OnChildRemove(Element* child)
{
	if (child == this && type)
		type->OnChildRemove();
}

// Handles the "click" event to toggle the control's checked status.
void ElementFormControlNumericInput::ProcessDefaultAction(Event& event)
{
	ElementFormControl::ProcessDefaultAction(event);
	if (type)
		type->ProcessDefaultAction(event);
}

bool ElementFormControlNumericInput::GetIntrinsicDimensions(Vector2f& dimensions, float& ratio)
{
	if (type)
		return type->GetIntrinsicDimensions(dimensions, ratio);
	return false;
}

} // namespace Rml
