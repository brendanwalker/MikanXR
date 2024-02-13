#include "IntPin.h"

// -- IntPin -----
IntPinBase::IntPinBase() : NodePin()
{
	setHasDefaultValue(true);
}

float IntPinBase::editorComputeInputWidth() const
{
	if (m_connectedLinks.size() == 0)
	{
		return ImGui::CalcTextSize(m_name.c_str()).x + 50.f + 11.f;
	}

	return NodePin::editorComputeInputWidth();
}

ImNodesPinShape IntPinBase::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(33, 227, 175, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(135, 239, 195, alpha * 255));

	return pinShape;	
}

void IntPinBase::editorRenderContextMenu(const NodeEditorState& editorState)
{
}

// -- IntPin -----
void IntPin::copyValueFromSourcePin()
{
	IntPinPtr sourcePin = std::dynamic_pointer_cast<IntPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void IntPin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		ImGui::InputInt("", &value, 0);
	}
}

// -- Int2Pin -----
void Int2Pin::copyValueFromSourcePin()
{
	Int2PinPtr sourcePin = std::dynamic_pointer_cast<Int2Pin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void Int2Pin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::InputInt2("", value.data());
	}
}

// -- Int3Pin -----
void Int3Pin::copyValueFromSourcePin()
{
	Int3PinPtr sourcePin = std::dynamic_pointer_cast<Int3Pin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void Int3Pin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150.0f);
		ImGui::InputInt3("", value.data());
	}
}

// -- Int4Pin -----
void Int4Pin::copyValueFromSourcePin()
{
	Int4PinPtr sourcePin = std::dynamic_pointer_cast<Int4Pin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void Int4Pin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputInt4("", value.data());
	}
}
