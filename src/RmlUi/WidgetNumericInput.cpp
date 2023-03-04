/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "WidgetNumericInput.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementScroll.h>
#include <RmlUi/Core/ElementText.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/GeometryUtilities.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Math.h>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/SystemInterface.h>
#include "ElementNumericSelection.h"

#include <algorithm>
#include <limits.h>

namespace Rml {

static constexpr float DEFAULT_REPEAT_DELAY = 0.5f;
static constexpr float DEFAULT_REPEAT_PERIOD = 0.1f;
static constexpr float CURSOR_BLINK_TIME = 0.7f;

static bool IsNumericCharacter(char c) {
	return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+';
}

WidgetNumericInput::WidgetNumericInput(ElementFormControl* _parent) 
	: internal_dimensions(0, 0)
	, scroll_offset(0, 0)
	, selection_geometry(_parent)
	, cursor_position(0, 0)
	, cursor_size(0, 0)
	, cursor_geometry(_parent)
{
	keyboard_showed = false;
	
	parent = _parent;
	parent->SetProperty(PropertyId::WhiteSpace, Property(Style::WhiteSpace::Pre));
	parent->SetProperty(PropertyId::OverflowX, Property(Style::Overflow::Hidden));
	parent->SetProperty(PropertyId::OverflowY, Property(Style::Overflow::Hidden));
	parent->SetProperty(PropertyId::Drag, Property(Style::Drag::Drag));
	parent->SetClientArea(Box::CONTENT);

	parent->AddEventListener(EventId::Keydown, this, true);
	parent->AddEventListener(EventId::Textinput, this, true);
	parent->AddEventListener(EventId::Focus, this, true);
	parent->AddEventListener(EventId::Blur, this, true);
	parent->AddEventListener(EventId::Mousedown, this, true);
	parent->AddEventListener(EventId::Dblclick, this, true);
	parent->AddEventListener(EventId::Drag, this, true);

	ElementPtr unique_text = Factory::InstanceElement(parent, "#text", "#text", XMLAttributes());
	text_element = rmlui_dynamic_cast< ElementText* >(unique_text.get());
	ElementPtr unique_selected_text = Factory::InstanceElement(parent, "#text", "#text", XMLAttributes());
	selected_text_element = rmlui_dynamic_cast< ElementText* >(unique_selected_text.get());
	if (text_element)
	{
		text_element->SuppressAutoLayout();
		parent->AppendChild(std::move(unique_text), false);

		selected_text_element->SuppressAutoLayout();
		parent->AppendChild(std::move(unique_selected_text), false);
	}

	// Create the dummy selection element.
	ElementPtr unique_selection = Factory::InstanceElement(parent, "#numeric_selection", "numeric_selection", XMLAttributes());
	if (ElementNumericSelection* text_selection_element = rmlui_dynamic_cast<ElementNumericSelection*>(unique_selection.get()))
	{
		selection_element = text_selection_element;
		text_selection_element->SetWidget(this);
		parent->AppendChild(std::move(unique_selection), false);
	}

	arrows[0] = nullptr;
	arrows[1] = nullptr;

	arrow_timers[0] = -1;
	arrow_timers[1] = -1;
	last_update_time = 0;

	edit_index = 0;
	absolute_cursor_index = 0;
	cursor_on_right_side_of_character = true;
	cancel_next_drag = false;

	ideal_cursor_position = 0;

	max_length = -1;

	selection_anchor_index = 0;
	selection_begin_index = 0;
	selection_length = 0;

	ShowCursor(false);
}

WidgetNumericInput::~WidgetNumericInput()
{
	parent->RemoveEventListener(EventId::Keydown, this, true);
	parent->RemoveEventListener(EventId::Textinput, this, true);
	parent->RemoveEventListener(EventId::Focus, this, true);
	parent->RemoveEventListener(EventId::Blur, this, true);
	parent->RemoveEventListener(EventId::Mousedown, this, true);
	parent->RemoveEventListener(EventId::Dblclick, this, true);
	parent->RemoveEventListener(EventId::Drag, this, true);

	// Remove all the children added by the text widget.
	parent->RemoveChild(text_element);
	parent->RemoveChild(selected_text_element);
	parent->RemoveChild(selection_element);

	for (int i = 0; i < 2; i++)
	{
		if (arrows[i] != nullptr)
		{
			arrows[i]->RemoveEventListener(EventId::Mousedown, this);
			arrows[i]->RemoveEventListener(EventId::Mouseup, this);
			arrows[i]->RemoveEventListener(EventId::Mouseout, this);
			parent->RemoveChild(arrows[i]);
		}
	}
}

// Initializes the slider to a given orientation.
bool WidgetNumericInput::Initialize()
{
	// Create all of our child elements as standard elements, and abort if we can't create them.
	ElementPtr arrow0_element = Factory::InstanceElement(parent, "*", "sliderarrowinc", XMLAttributes());
	ElementPtr arrow1_element = Factory::InstanceElement(parent, "*", "sliderarrowdec", XMLAttributes());

	if (!arrow0_element || !arrow1_element)
	{
		return false;
	}

	// Add them as non-DOM elements.
	arrows[0] = parent->AppendChild(std::move(arrow0_element), false);
	arrows[1] = parent->AppendChild(std::move(arrow1_element), false);

	return true;
}

// Sets the value of the text field.
void WidgetNumericInput::SetValue(const String& value)
{
	text_element->SetText(value);
	FormatElement();

	ResetEditCursorAndUpdateCursorPosition();
}

// Sets the maximum length (in characters) of this text field.
void WidgetNumericInput::SetMaxLength(int _max_length)
{
	if (max_length != _max_length)
	{
		max_length = _max_length;
		if (max_length >= 0)
		{
			String value = GetElement()->GetAttribute< String >("value", "");

			int num_characters = 0;
			size_t i_erase = value.size();

			for (auto it = StringIteratorU8(value); it; ++it)
			{
				num_characters += 1;
				if (num_characters > max_length)
				{
					i_erase = size_t(it.offset());
					break;
				}
			}

			if(i_erase < value.size())
			{
				value.erase(i_erase);
				GetElement()->SetAttribute("value", value);
			}
		}
	}
}

// Returns the maximum length (in characters) of this text field.
int WidgetNumericInput::GetMaxLength() const
{
	return max_length;
}

int WidgetNumericInput::GetLength() const
{
	const String value = text_element->GetText();
	size_t result = StringUtilities::LengthUTF8(value);
	return (int)result;
}

// Update the colours of the selected text.
void WidgetNumericInput::UpdateSelectionColours()
{
	// Determine what the colour of the selected text is. If our 'selection' element has the 'color'
	// attribute set, then use that. Otherwise, use the inverse of our own text colour.
	Colourb colour;
	const Property* colour_property = selection_element->GetLocalProperty("color");
	if (colour_property != nullptr)
		colour = colour_property->Get< Colourb >();
	else
	{
		colour = parent->GetComputedValues().color();
		colour.red = 255 - colour.red;
		colour.green = 255 - colour.green;
		colour.blue = 255 - colour.blue;
	}

	// Set the computed text colour on the element holding the selected text.
	selected_text_element->SetProperty(PropertyId::Color, Property(colour, Property::COLOUR));

	// If the 'background-color' property has been set on the 'selection' element, use that as the
	// background colour for the selected text. Otherwise, use the inverse of the selected text
	// colour.
	colour_property = selection_element->GetLocalProperty("background-color");
	if (colour_property != nullptr)
		selection_colour = colour_property->Get< Colourb >();
	else
		selection_colour = Colourb(255 - colour.red, 255 - colour.green, 255 - colour.blue, colour.alpha);

	// Color may have changed, so we update the cursor geometry.
	GenerateCursor();
}

// Updates the cursor, if necessary.
void WidgetNumericInput::OnUpdate()
{
	double current_time = GetSystemInterface()->GetElapsedTime();

	if (cursor_timer > 0)
	{
		cursor_timer -= float(current_time - last_update_time);
		last_update_time = current_time;

		while (cursor_timer <= 0)
		{
			cursor_timer += CURSOR_BLINK_TIME;
			cursor_visible = !cursor_visible;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		bool updated_time = false;
		float delta_time = 0;

		if (arrow_timers[i] > 0)
		{
			if (!updated_time)
			{
				delta_time = float(current_time - last_update_time);
				last_update_time = current_time;
			}

			arrow_timers[i] -= delta_time;
			while (arrow_timers[i] <= 0)
			{
				arrow_timers[i] += DEFAULT_REPEAT_PERIOD;
				if (i == 0)
					OnValueDecrement();
				else
					OnValueIncrement();
			}
		}
	}
}

void WidgetNumericInput::OnResize()
{
	GenerateCursor();

	Vector2f text_position = parent->GetBox().GetPosition(Box::CONTENT);
	text_element->SetOffset(text_position, parent);
	selected_text_element->SetOffset(text_position, parent);

	Vector2f new_internal_dimensions = parent->GetBox().GetSize(Box::CONTENT);
	if (new_internal_dimensions != internal_dimensions)
	{
		internal_dimensions = new_internal_dimensions;

		FormatElement();
		UpdateCursorPosition();
	}
}

// Renders the cursor, if it is visible.
void WidgetNumericInput::OnRender()
{
	ElementUtilities::SetClippingRegion(text_element);

	Vector2f text_translation = parent->GetAbsoluteOffset() - Vector2f(parent->GetScrollLeft(), parent->GetScrollTop());
	selection_geometry.Render(text_translation);

	if (cursor_visible &&
		!parent->IsDisabled())
	{
		cursor_geometry.Render(text_translation + cursor_position);
	}
}

// Formats the widget's internal content.
void WidgetNumericInput::OnLayout()
{
	FormatElement();
	parent->SetScrollLeft(scroll_offset.x);
	parent->SetScrollTop(scroll_offset.y);
}

// Returns the input element's underlying text element.
ElementText* WidgetNumericInput::GetTextElement()
{
	return text_element;
}

/// Returns the input element's underlying text element.
const ElementText* WidgetNumericInput::GetTextElementConst() const
{
	return text_element;
}

// Returns the input element's maximum allowed text dimensions.
Vector2f WidgetNumericInput::GetTextDimensions() const
{
	return internal_dimensions;
}

// Gets the parent element containing the widget.
Element* WidgetNumericInput::GetElement() const
{
	return parent;
}

// Dispatches a change event to the widget's element.
void WidgetNumericInput::DispatchChangeEvent()
{
	String contentValue= text_element->GetText();
	
	Dictionary parameters;
	parameters["value"] = contentValue;
	GetElement()->DispatchEvent(EventId::Change, parameters);
}

void WidgetNumericInput::DispatchSubmitEvent()
{
	String contentValue = text_element->GetText();

	Dictionary parameters;
	parameters["value"] = contentValue;
	GetElement()->DispatchEvent(EventId::Submit, parameters);
}


// Processes the "keydown" and "textinput" event to write to the input field, and the "focus" and "blur" to set
// the state of the cursor.
void WidgetNumericInput::ProcessEvent(Event& event)
{
	if (parent->IsDisabled())
		return;

	switch (event.GetId())
	{
	case EventId::Keydown:
	{
		Input::KeyIdentifier key_identifier = (Input::KeyIdentifier) event.GetParameter< int >("key_identifier", 0);
		bool numlock = event.GetParameter< int >("num_lock_key", 0) > 0;
		bool shift = event.GetParameter< int >("shift_key", 0) > 0;
		bool ctrl = event.GetParameter< int >("ctrl_key", 0) > 0;

		switch (key_identifier)
		{
		case Input::KI_NUMPAD4:	if (numlock) break; //-fallthrough
		case Input::KI_LEFT:		MoveCursorHorizontal(CursorMovement::Left, shift); break;

		case Input::KI_NUMPAD6:	if (numlock) break; //-fallthrough
		case Input::KI_RIGHT:		MoveCursorHorizontal(CursorMovement::Right, shift); break;

		case Input::KI_NUMPAD7:	if (numlock) break; //-fallthrough
		case Input::KI_HOME:		MoveCursorHorizontal(CursorMovement::Begin, shift); break;

		case Input::KI_NUMPAD1:	if (numlock) break; //-fallthrough
		case Input::KI_END:		MoveCursorHorizontal(CursorMovement::End, shift); break;

		case Input::KI_UP:
		case Input::KI_NUMPAD8:
			OnValueIncrement();
			break;

		case Input::KI_DOWN:
		case Input::KI_NUMPAD2:
			OnValueDecrement();
			break;

		case Input::KI_BACK:
		{
			CursorMovement direction = (CursorMovement::Left);
			if (DeleteCharacters(direction))
			{
				FormatElement();
				ResetEditCursorAndUpdateCursorPosition();
			}

			ShowCursor(true);
		}
		break;

		case Input::KI_DECIMAL:	if (numlock) break; //-fallthrough
		case Input::KI_DELETE:
		{
			CursorMovement direction = (CursorMovement::Right);
			if (DeleteCharacters(direction))
			{
				FormatElement();
				ResetEditCursorAndUpdateCursorPosition();
			}

			ShowCursor(true);
		}
		break;

		case Input::KI_NUMPADENTER:
		case Input::KI_RETURN:
		{
			LineBreak();
		}
		break;

		case Input::KI_A:
		{
			if (ctrl)
			{
				MoveCursorHorizontal(CursorMovement::Begin, false);
				MoveCursorHorizontal(CursorMovement::End, true);
			}
		}
		break;

		case Input::KI_C:
		{
			if (ctrl && selection_length > 0)
				CopySelection();
		}
		break;

		case Input::KI_X:
		{
			if (ctrl && selection_length > 0)
			{
				CopySelection();
				DeleteSelection();
			}
		}
		break;

		case Input::KI_V:
		{
			if (ctrl)
			{
				String clipboard_text;
				GetSystemInterface()->GetClipboardText(clipboard_text);

				AddCharacters(clipboard_text);
			}
		}
		break;

		// Ignore tabs so input fields can be navigated through with keys.
		case Input::KI_TAB:
			return;

		default:
		break;
		}

		event.StopPropagation();
	}
	break;

	case EventId::Textinput:
	{
		// Only process the text if no modifier keys are pressed.
		if (event.GetParameter< int >("ctrl_key", 0) == 0 &&
			event.GetParameter< int >("alt_key", 0) == 0 &&
			event.GetParameter< int >("meta_key", 0) == 0)
		{
			String text = event.GetParameter("text", String{});
			AddCharacters(text);
		}

		ShowCursor(true);
		event.StopPropagation();
	}
	break;
	case EventId::Focus:
	{
		if (event.GetTargetElement() == parent)
		{
			UpdateSelection(false);
			ShowCursor(true, false);
		}
	}
	break;
	case EventId::Blur:
	{
		if (event.GetTargetElement() == parent)
		{
			ClearSelection();
			ShowCursor(false, false);
			LineBreak();
		}
	}
	break;
	case EventId::Drag:
		if (cancel_next_drag)
		{
			// We currently ignore drag events right after a double click. They would need to be handled
			// specially by selecting whole words at a time, which is not yet implemented.
			break;
		}
		//-fallthrough
	case EventId::Mousedown:
	{
		if (event.GetTargetElement() == parent)
		{
			Vector2f mouse_position = Vector2f(event.GetParameter< float >("mouse_x", 0), event.GetParameter< float >("mouse_y", 0));
			mouse_position -= text_element->GetAbsoluteOffset();

			//cursor_character_index = CalculateCharacterIndex(mouse_position.x);
			absolute_cursor_index = CalculateCharacterIndex(mouse_position.x);

			ResetEditCursor();
			MoveCursorToCharacterBoundaries(false);

			UpdateCursorPosition();
			ideal_cursor_position = cursor_position.x;

			UpdateSelection(event == EventId::Drag || event.GetParameter< int >("shift_key", 0) > 0);

			ShowCursor(true); 
			cancel_next_drag = false;
		}
		else if (event.GetTargetElement() == arrows[0])
		{
			arrow_timers[0] = DEFAULT_REPEAT_DELAY;
			last_update_time = GetSystemInterface()->GetElapsedTime();
			OnValueIncrement();
		}
		else if (event.GetTargetElement() == arrows[1])
		{
			arrow_timers[1] = DEFAULT_REPEAT_DELAY;
			last_update_time = GetSystemInterface()->GetElapsedTime();
			OnValueDecrement();
		}
	}
	break;
	case EventId::Dblclick:
	{
		if (event.GetTargetElement() == parent)
		{
			ExpandSelection();
			cancel_next_drag = true;
		}
	}
	break;

	default:
		break;
	}

}

// Adds a new character to the string at the cursor position.
bool WidgetNumericInput::AddCharacters(String string)
{
	// Erase invalid characters from string
	auto invalid_character = [this](char c) {
		return ((unsigned char)c <= 127 && !IsCharacterValid(c));
	};
	string.erase(
		std::remove_if(string.begin(), string.end(), invalid_character),
		string.end()
	);

	if (string.empty())
		return false;

	if (selection_length > 0)
		DeleteSelection();

	if (max_length >= 0 && GetLength() >= max_length)
		return false;

	//String value = GetElement()->GetAttribute< String >("value", "");
	String value = text_element->GetText();
	
	value.insert(std::min<size_t>(GetCursorIndex(), value.size()), string);

	edit_index += (int)string.size();

	SetValue(value);
	DispatchChangeEvent();

	UpdateSelection(false);

	return true;
}

// Deletes a character from the string.
bool WidgetNumericInput::DeleteCharacters(CursorMovement direction)
{
	// We set a selection of characters according to direction, and then delete it.
	// If we already have a selection, we delete that first.
	if (selection_length <= 0)
		MoveCursorHorizontal(direction, true);

	if (selection_length > 0)
	{
		DeleteSelection();
		DispatchChangeEvent();

		UpdateSelection(false);

		return true;
	}

	return false;
}

void WidgetNumericInput::LineBreak()
{	
	const String& textElementValue = text_element->GetText();

	if (IsNumericContentValid(textElementValue))
	{
		// Commit new value to the "value" Attribute
		// This will get routed back to ourselves via InputTypeNumber::OnAttributeChange()
		GetElement()->SetAttribute("value", textElementValue);

		// Tell event listeners that a value was committed
		DispatchSubmitEvent();
	}
	else
	{
		// Restore the text element back to the attribute value
		const String attribValue = GetElement()->GetAttribute< String >("value", "");

		SetValue(attribValue);
	}
}

// Copies the selection (if any) to the clipboard.
void WidgetNumericInput::CopySelection()
{
	const String value = text_element->GetText();
	const String snippet = value.substr(std::min(size_t(selection_begin_index), size_t(value.size())), selection_length);
	GetSystemInterface()->SetClipboardText(snippet);
}

// Returns the absolute index of the cursor.
int WidgetNumericInput::GetCursorIndex() const
{
	return edit_index;
}

// Moves the cursor along the current line.
void WidgetNumericInput::MoveCursorHorizontal(CursorMovement movement, bool select)
{
	// Whether to seek forward or back to align to utf8 boundaries later.
	bool seek_forward = false;

	switch (movement)
	{
	case CursorMovement::Begin:
		absolute_cursor_index = 0;
		break;
	case CursorMovement::Left:
		if (!select && selection_length > 0)
			absolute_cursor_index = selection_begin_index;
		else
			absolute_cursor_index -= 1;
		break;
	case CursorMovement::Right:
		seek_forward = true;
		if (!select && selection_length > 0)
			absolute_cursor_index = selection_begin_index + selection_length;
		else
			absolute_cursor_index += 1;
		break;
	case CursorMovement::End:
		absolute_cursor_index = INT_MAX;
		break;
	default:
		break;
	}
	
	absolute_cursor_index = Math::Max(0, absolute_cursor_index);

	ResetEditCursorAndUpdateCursorPosition();
	MoveCursorToCharacterBoundaries(seek_forward);

	ideal_cursor_position = cursor_position.x;
	UpdateSelection(select);
	ShowCursor(true);
}

void WidgetNumericInput::MoveCursorToCharacterBoundaries(bool forward)
{
	const String& line= text_element->GetText();
	const char* p_line_begin = line.data();
	const char* p_line_end = p_line_begin + line.size();
	const char* p_cursor = p_line_begin + absolute_cursor_index;
	const char* p = p_cursor;

	if (forward)
		p = StringUtilities::SeekForwardUTF8(p_cursor, p_line_end);
	else
		p = StringUtilities::SeekBackwardUTF8(p_cursor, p_line_begin);

	if (p != p_cursor)
	{
		absolute_cursor_index += int(p - p_cursor);
	}
	UpdateCursorPosition();
}

void WidgetNumericInput::ExpandSelection()
{
	const String& line= text_element->GetText();
	const char* const p_begin = line.data();
	const char* const p_end = p_begin + line.size();
	const char* const p_index = p_begin + absolute_cursor_index;

	// If true, we are expanding word characters, if false, whitespace characters.
	// The first character encountered defines the bool.
	bool expanding_word = false;
	bool expanding_word_set = false;

	auto character_is_wrong_type = [&expanding_word_set, &expanding_word](const char* p) -> bool {
		bool is_numeric_character = IsNumericCharacter(*p);
		if (expanding_word_set && (expanding_word != is_numeric_character))
			return true;
		if (!expanding_word_set)
		{
			expanding_word = is_numeric_character;
			expanding_word_set = true;
		}
		return false;
	};

	auto search_left = [&]() -> const char* {
		const char* p = p_index;
		for (; p > p_begin; p--)
			if (character_is_wrong_type(p - 1))
				break;
		return p;
	};
	auto search_right = [&]() -> const char* {
		const char* p = p_index;
		for (; p < p_end; p++)
			if (character_is_wrong_type(p))
				break;
		return p;
	};

	const char* p_left = p_index;
	const char* p_right = p_index;

	if (cursor_on_right_side_of_character)
	{
		p_right = search_right();
		p_left = search_left();
	}
	else
	{
		p_left = search_left();
		p_right = search_right();
	}

	absolute_cursor_index -= int(p_index - p_left);
	ResetEditCursorAndUpdateCursorPosition();
	MoveCursorToCharacterBoundaries(false);
	UpdateSelection(false);

	absolute_cursor_index += int(p_right - p_left);
	ResetEditCursorAndUpdateCursorPosition();
	MoveCursorToCharacterBoundaries(true);
	UpdateSelection(true);
}

// Updates the absolute cursor index from the relative cursor indices.
void WidgetNumericInput::ResetEditCursor()
{
	edit_index = absolute_cursor_index;
}

// Updates the relative cursor indices from the absolute cursor index.
void WidgetNumericInput::ResetEditCursorAndUpdateCursorPosition()
{
	edit_index = absolute_cursor_index;

	UpdateCursorPosition();
}

// Calculates the character index along a line under a specific horizontal position.
int WidgetNumericInput::CalculateCharacterIndex(float position)
{
	int prev_offset = 0;
	float prev_line_width = 0;

	cursor_on_right_side_of_character = true;

	const String& line= text_element->GetText();
	for(auto it = StringIteratorU8(line, 0, line.size()); it; )
	{
		++it;
		int offset = (int)it.offset();

		float line_width = (float) ElementUtilities::GetStringWidth(text_element, line.substr(0, offset));
		if (line_width > position)
		{
			if (position - prev_line_width < line_width - position)
			{
				return prev_offset;
			}
			else
			{
				cursor_on_right_side_of_character = false;
				return offset;
			}
		}

		prev_line_width = line_width;
		prev_offset = offset;
	}

	return prev_offset;
}

// Shows or hides the cursor.
void WidgetNumericInput::ShowCursor(bool show, bool move_to_cursor)
{
	if (show)
	{
		cursor_visible = true;
		SetKeyboardActive(true);
		keyboard_showed = true;
		
		cursor_timer = CURSOR_BLINK_TIME;
		last_update_time = GetSystemInterface()->GetElapsedTime();

		// Shift the cursor into view.
		if (move_to_cursor)
		{
			float minimum_scroll_top = (cursor_position.y + cursor_size.y) - parent->GetClientHeight();
			if (parent->GetScrollTop() < minimum_scroll_top)
				parent->SetScrollTop(minimum_scroll_top);
			else if (parent->GetScrollTop() > cursor_position.y)
				parent->SetScrollTop(cursor_position.y);

			float minimum_scroll_left = (cursor_position.x + cursor_size.x) - parent->GetClientWidth();
			if (parent->GetScrollLeft() < minimum_scroll_left)
				parent->SetScrollLeft(minimum_scroll_left);
			else if (parent->GetScrollLeft() > cursor_position.x)
				parent->SetScrollLeft(cursor_position.x);

			scroll_offset.x = parent->GetScrollLeft();
			scroll_offset.y = parent->GetScrollTop();
		}
	}
	else
	{
		cursor_visible = false;
		cursor_timer = -1;
		last_update_time = 0;
		if (keyboard_showed)
		{
			SetKeyboardActive(false);
			keyboard_showed = false;
		}
	}
}

// Formats the element, laying out the text and inserting scrollbars as appropriate.
void WidgetNumericInput::FormatElement()
{
	using namespace Style;
	Rml::Vector2f parentBoxSize = parent->GetBox().GetSize(Box::PADDING);

	// Format the text and determine its total area.
	Vector2f content_area = FormatText();

	parent->SetContentBox(Vector2f(0, 0), content_area);

	// Now we size the arrows.
	for (int i = 0; i < 2; i++)
	{
		Box arrow_box;
		ElementUtilities::BuildBox(arrow_box, parentBoxSize, arrows[i]);

		// Clamp the size to (0, 0).
		Vector2f arrow_size = arrow_box.GetSize();
		if (arrow_size.x < 0 ||
			arrow_size.y < 0)
			arrow_box.SetContent(Vector2f(0, 0));

		arrows[i]->SetBox(arrow_box);
	}

	{
		const float rightPadding = parent->GetBox().GetEdge(Box::PADDING, Box::RIGHT);
		const Vector2f offset(parentBoxSize.x - rightPadding, 0.f);

		arrows[0]->SetOffset(offset, parent);
	}

	{
		const float rightPadding = parent->GetBox().GetEdge(Box::PADDING, Box::RIGHT);
		const float arrowContentHeight = arrows[1]->GetBox().GetSize(Box::CONTENT).y;
		const Vector2f offset(parentBoxSize.x - rightPadding, parentBoxSize.y - arrowContentHeight);

		arrows[1]->SetOffset(offset, parent);
	}

	if (parent->IsDisabled())
	{
		// Propagate disabled state to child elements
		arrows[0]->SetPseudoClass("disabled", true);
		arrows[1]->SetPseudoClass("disabled", true);
	}
}

// Formats the input element's text field.
Vector2f WidgetNumericInput::FormatText()
{
	absolute_cursor_index = edit_index;

	Vector2f content_area(0, 0);

	// Clear the old lines, and all the lines in the text elements.
	text_element->ClearLines();
	selected_text_element->ClearLines();

	// Clear the selection background geometry, and get the vertices and indices so the new geo can
	// be generated.
	selection_geometry.Release(true);
	Vector< Vertex >& selection_vertices = selection_geometry.GetVertices();
	Vector< int >& selection_indices = selection_geometry.GetIndices();

	// Determine the line-height of the text element.
	float line_height = parent->GetLineHeight();

	int line_begin = 0;
	Vector2f line_position(0, 0);

	// Generate the next line.
	//String& line, int& line_length, float& line_width, int line_begin, float maximum_line_width, float right_spacing_width, bool trim_whitespace_prefix, bool decode_escape_characters
	String line;
	int line_length; 
	float line_width;
	text_element->GenerateLine(
		line, line_length, line_width, 
		line_begin, parent->GetClientWidth() - cursor_size.x, 0, true, false);


	// Now that we have the string of characters appearing on the new line, we split it into
	// three parts; the unselected text appearing before any selected text on the line, the
	// selected text on the line, and any unselected text after the selection.
	String pre_selection, selection, post_selection;
	GetLineSelection(pre_selection, selection, post_selection, line, line_begin);

	// The pre-selected text is placed, if there is any (if the selection starts on or before
	// the beginning of this line, then this will be empty).
	if (!pre_selection.empty())
	{
		text_element->AddLine(line_position, pre_selection);
		line_position.x += ElementUtilities::GetStringWidth(text_element, pre_selection);
	}

	// Return the extra kerning that would result in joining two strings.
	auto GetKerningBetween = [this](const String& left, const String& right) -> float {
		if (left.empty() || right.empty())
			return 0.0f;
		// We could join the whole string, and compare the result of the joined width to the individual widths of each string. Instead, we just take the
		// two neighboring characters from each string and compare the string width with and without kerning, which should be much faster.
		const Character left_back = StringUtilities::ToCharacter(StringUtilities::SeekBackwardUTF8(&left.back(), &left.front()));
		const String right_front_u8 = right.substr(0, size_t(StringUtilities::SeekForwardUTF8(right.c_str() + 1, right.c_str() + right.size()) - right.c_str()));
		const int width_kerning = ElementUtilities::GetStringWidth(text_element, right_front_u8, left_back);
		const int width_no_kerning = ElementUtilities::GetStringWidth(text_element, right_front_u8, Character::Null);
		return float(width_kerning - width_no_kerning);
	};

	// If there is any selected text on this line, place it in the selected text element and
	// generate the geometry for its background.
	if (!selection.empty())
	{
		line_position.x += GetKerningBetween(pre_selection, selection);
		selected_text_element->AddLine(line_position, selection);
		const int selection_width = ElementUtilities::GetStringWidth(selected_text_element, selection);

		selection_vertices.resize(selection_vertices.size() + 4);
		selection_indices.resize(selection_indices.size() + 6);
		GeometryUtilities::GenerateQuad(
			&selection_vertices[selection_vertices.size() - 4], 
			&selection_indices[selection_indices.size() - 6], 
			line_position, 
			Vector2f((float)selection_width, line_height), 
			selection_colour, 
			(int)selection_vertices.size() - 4);

		line_position.x += selection_width;
	}

	// If there is any unselected text after the selection on this line, place it in the
	// standard text element after the selected text.
	if (!post_selection.empty())
	{
		line_position.x += GetKerningBetween(selection, post_selection);
		text_element->AddLine(line_position, post_selection);
	}

	// Grow the content area width-wise if this line is the longest so far, and push the
	// height out.
	content_area.x = Math::Max(content_area.x, line_width + cursor_size.x);
	content_area.y = line_position.y;

	return content_area;
}

// Generates the text cursor.
void WidgetNumericInput::GenerateCursor()
{
	// Generates the cursor.
	cursor_geometry.Release();

	Vector< Vertex >& vertices = cursor_geometry.GetVertices();
	vertices.resize(4);

	Vector< int >& indices = cursor_geometry.GetIndices();
	indices.resize(6);

	cursor_size.x = Math::RoundFloat( ElementUtilities::GetDensityIndependentPixelRatio(text_element) );
	cursor_size.y = text_element->GetLineHeight() + 2.0f;

	Colourb color = parent->GetComputedValues().color();

	if (const Property* property = parent->GetProperty(PropertyId::CaretColor))
	{
		if (property->unit == Property::COLOUR)
			color = property->Get<Colourb>();
	}

	GeometryUtilities::GenerateQuad(&vertices[0], &indices[0], Vector2f(0, 0), cursor_size, color);
}

void WidgetNumericInput::UpdateCursorPosition()
{
	if (text_element->GetFontFaceHandle() == 0)
		return;

	//cursor_position.x = (float) ElementUtilities::GetStringWidth(text_element, numberContent.content.substr(0, cursor_character_index));
	const String& content = text_element->GetText();
	cursor_position.x = (float) ElementUtilities::GetStringWidth(text_element, content.substr(0, absolute_cursor_index));
	cursor_position.y = -1.f;
}

// Expand the text selection to the position of the cursor.
void WidgetNumericInput::UpdateSelection(bool selecting)
{
	if (!selecting)
	{
		selection_anchor_index = edit_index;
		ClearSelection();
	}
	else
	{
		int new_begin_index;
		int new_end_index;

		if (edit_index > selection_anchor_index)
		{
			new_begin_index = selection_anchor_index;
			new_end_index = edit_index;
		}
		else
		{
			new_begin_index = edit_index;
			new_end_index = selection_anchor_index;
		}

		if (new_begin_index != selection_begin_index ||
			new_end_index - new_begin_index != selection_length)
		{
			selection_begin_index = new_begin_index;
			selection_length = new_end_index - new_begin_index;

			FormatText();
		}
	}
}

// Removes the selection of text.
void WidgetNumericInput::ClearSelection()
{
	if (selection_length > 0)
	{
		selection_length = 0;
		FormatElement();
	}
}

// Deletes all selected text and removes the selection.
void WidgetNumericInput::DeleteSelection()
{
	if (selection_length > 0)
	{
		// Update display content, but don't commit to "value" attribute 
		const String& value = text_element->GetText();
		const String new_value = value.substr(0, selection_begin_index) + value.substr(std::min(size_t(selection_begin_index + selection_length), size_t(value.size())));
		SetValue(new_value);

		// Move the cursor to the beginning of the old selection.
		absolute_cursor_index = selection_begin_index;
		ResetEditCursorAndUpdateCursorPosition();

		// Erase our record of the selection.
		ClearSelection();
	}
}

// Split one line of text into three parts, based on the current selection.
void WidgetNumericInput::GetLineSelection(String& pre_selection, String& selection, String& post_selection, const String& line, int line_begin)
{
	// Check if we have any selection at all, and if so if the selection is on this line.
	if (selection_length <= 0 ||
		selection_begin_index + selection_length < line_begin ||
		selection_begin_index > line_begin + (int) line.size())
	{
		pre_selection = line;
		return;
	}

	int line_length = (int)line.size();
	using namespace Math;

	// Split the line up into its three parts, depending on the size and placement of the selection.
	pre_selection = line.substr(0, Max(0, selection_begin_index - line_begin));
	selection = line.substr(Clamp(selection_begin_index - line_begin, 0, line_length), Max(0, selection_length + Min(0, selection_begin_index - line_begin)));
	post_selection = line.substr(Clamp(selection_begin_index + selection_length - line_begin, 0, line_length));
}

void WidgetNumericInput::SetKeyboardActive(bool active)
{
	SystemInterface* system = GetSystemInterface();
	if (system) {
		if (active) 
		{
			system->ActivateKeyboard();
		} else 
		{
			system->DeactivateKeyboard();
		}
	}
}
	
} // namespace Rml
