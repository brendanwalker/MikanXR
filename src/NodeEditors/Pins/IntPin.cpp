#include "IntPin.h"

// -- IntPin -----
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

void IntPinBase::editorRenderContextMenu(class NodeEditorState* editorState)
{
}

// -- IntPin -----
void IntPin::editorRenderInputTextEntry(class NodeEditorState* editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		ImGui::InputInt("", &value, 0);
	}
}

void IntPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(33, 227, 175, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(135, 239, 195, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(135, 239, 195, 255));
}

ImU32 IntPin::editorGetLinkStyleColor() const
{
	return IM_COL32(33, 227, 175, 255);
}

// -- Int2Pin -----
void Int2Pin::editorRenderInputTextEntry(class NodeEditorState* editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		ImGui::InputInt2("", value);
	}
}

// -- Int3Pin -----
void Int3Pin::editorRenderInputTextEntry(class NodeEditorState* editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(150.0f);
		ImGui::InputInt3("", value);
	}
}

// -- Int4Pin -----
void Int4Pin::editorRenderInputTextEntry(class NodeEditorState* editorState)
{
	if (m_connectedLinks.size() == 0)
	{
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputInt4("", value);
	}
}
