#include "FloatPin.h"

// -- FloatPinBase -----
FloatPinBase::FloatPinBase() : NodePin()
{
	setHasDefaultValue(true);
}

float FloatPinBase::editorComputeInputWidth() const
{
	if (m_connectedLinks.size() == 0)
	{
		return ImGui::CalcTextSize(m_name.c_str()).x + 50.f + 11.0f;
	}

	return NodePin::editorComputeInputWidth();
}

ImNodesPinShape FloatPinBase::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(156, 253, 65, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(144, 225, 137, alpha * 255));

	return pinShape;	
}

void FloatPinBase::editorRenderContextMenu(const NodeEditorState& editorState)
{
}

// -- FloatPin -----
void FloatPin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		ImGui::InputFloat("", &value);
	}
}

void FloatPin::copyValueFromSourcePin()
{
	FloatPinPtr sourcePin= std::dynamic_pointer_cast<FloatPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void FloatPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(156, 253, 65, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(144, 225, 137, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(144, 225, 137, 255));
}

ImU32 FloatPin::editorGetLinkStyleColor() const
{
	return IM_COL32(156, 253, 65, 255);
}

// -- Float2Pin -----
void Float2Pin::copyValueFromSourcePin()
{
	Float2PinPtr sourcePin = std::dynamic_pointer_cast<Float2Pin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void Float2Pin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::InputFloat2("", value.data());
	}
}

// -- Float3Pin -----
void Float3Pin::copyValueFromSourcePin()
{
	Float3PinPtr sourcePin = std::dynamic_pointer_cast<Float3Pin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void Float3Pin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150.0f);
		ImGui::InputFloat3("", value.data());
	}
}

// -- Float4Pin -----
void Float4Pin::copyValueFromSourcePin()
{
	Float4PinPtr sourcePin = std::dynamic_pointer_cast<Float4Pin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

void Float4Pin::editorRenderInputTextEntry(const NodeEditorState& editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputFloat4("", value.data());
	}
}
