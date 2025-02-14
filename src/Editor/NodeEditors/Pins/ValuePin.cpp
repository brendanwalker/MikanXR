#include "ValuePin.h"
#include "NodeEditorUI.h"

// -- IntPin -----
float ValuePin::editorComputeInputWidth() const
{
	if (m_connectedLinks.size() == 0)
	{
		return ImGui::CalcTextSize(m_name.c_str()).x + 50.f + 11.f;
	}

	return NodePin::editorComputeInputWidth();
}

ImNodesPinShape ValuePin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, editorValuePinColor(alpha));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, ImGui::GetColorU32(NodeEditorUI::getPinHoveredColor(alpha)));

	return pinShape;
}

const ImU32 ValuePin::editorValuePinColor(float alpha) const
{
	return ImGui::GetColorU32(NodeEditorUI::getPropertyColor(alpha));
}