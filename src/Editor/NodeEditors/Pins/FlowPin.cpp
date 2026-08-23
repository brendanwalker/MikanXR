#include "FlowPin.h"

FlowPin::FlowPin()
	: NodePin()
{
	m_bEditorShowPinName= false;
}

ImNodesPinShape FlowPin::editorComputePinShape() const
{
	if (m_connectedLinks.size() > 0)
		return ImNodesPinShape_TriangleFilled;
	else
		return ImNodesPinShape_Triangle;
}

std::shared_ptr<MkNodesScopedColorStyle> FlowPin::editorRenderMakePinStyle(float alpha)
{
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Pin, IM_COL32(225, 225, 225, (unsigned char)(alpha * 255)))
		.push(ImNodesCol_PinHovered, IM_COL32(255, 255, 255, (unsigned char)(alpha * 255)));
	return style;
}

std::shared_ptr<MkNodesScopedColorStyle> FlowPin::editorRenderMakeLinkStyle(float alpha)
{
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Link, IM_COL32(225, 225, 225, (unsigned char)alpha))
		.push(ImNodesCol_LinkHovered, IM_COL32(255, 255, 255, (unsigned char)alpha))
		.push(ImNodesCol_LinkSelected, IM_COL32(255, 255, 255, 255));
	return style;
}

void FlowPin::editorRenderContextMenu(const NodeEditorState& editorState) {}

ImU32 FlowPin::editorGetLinkStyleColor() const { return IM_COL32(225, 225, 225, 255); }