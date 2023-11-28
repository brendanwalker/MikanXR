#include "FlowPin.h"

ImNodesPinShape FlowPin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
	{
		pinShape = ImNodesPinShape_TriangleFilled;
	}

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(225, 225, 225, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(255, 255, 255, alpha * 255));

	return pinShape;	
}

void FlowPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(225, 225, 225, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(255, 255, 255, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(255, 255, 255, 255));
}

void FlowPin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}

ImU32 FlowPin::editorGetLinkStyleColor() const
{
	return IM_COL32(225, 225, 225, 255);
}