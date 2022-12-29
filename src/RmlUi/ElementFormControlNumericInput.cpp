
#include "ElementFormControlNumericInput.h"
#include "InputTypeNumber.h"
#include <RmlUi/Core/Event.h>

namespace Rml {

// Constructs a new ElementFormControlNumericInput.
ElementFormControlNumericInput::ElementFormControlNumericInput(const String& tag) : ElementFormControlInput(tag)
{
	// OnAttributeChange will be called right after this, possible with a non-default type. Thus,
	// creating the default InputTypeText here may result in it being destroyed in just a few moments.
	// Instead, we create the InputTypeText in OnAttributeChange in the case where the type attribute has not been set.
}

ElementFormControlNumericInput::~ElementFormControlNumericInput()
{}

// Checks for necessary functional changes in the control as a result of changed attributes.
void ElementFormControlNumericInput::OnAttributeChange(const ElementAttributes& changed_attributes)
{
	ElementFormControl::OnAttributeChange(changed_attributes);

	String new_type_name;

	auto it_type = changed_attributes.find("type");
	if (it_type != changed_attributes.end())
	{
		new_type_name = it_type->second.Get<String>("text");
	}

	if (!input_type || (!new_type_name.empty() && new_type_name != type_name))
	{
		if (new_type_name == "float")
		{
			input_type = MakeUnique<InputTypeNumber>(this, InputTypeNumber::FLOAT);
		}
		else
		{
			new_type_name = "int";
			input_type = MakeUnique<InputTypeNumber>(this, InputTypeNumber::INT);
		}

		if (!type_name.empty())
		{
			SetClass(type_name, false);
		}
		SetClass(new_type_name, true);
		type_name = new_type_name;

		DirtyLayout();
	}

	RMLUI_ASSERT(input_type);

	if (!input_type->OnAttributeChange(changed_attributes))
	{
		DirtyLayout();
	}
}
} // namespace Rml
