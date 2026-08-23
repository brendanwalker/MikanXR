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

ImNodesPinShape ValuePin::editorComputePinShape() const
{
	if (m_connectedLinks.size() > 0)
		return ImNodesPinShape_CircleFilled;
	else
		return ImNodesPinShape_Circle;
}

std::shared_ptr<MkNodesScopedColorStyle> ValuePin::editorRenderMakePinStyle(float alpha)
{
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Pin, editorValuePinColor(alpha))
		.push(ImNodesCol_PinHovered, ImGui::GetColorU32(NodeEditorUI::getPinHoveredColor(alpha)));
	return style;
}

const ImU32 ValuePin::editorValuePinColor(float alpha) const
{
	return ImGui::GetColorU32(NodeEditorUI::getPropertyColor(alpha));
}