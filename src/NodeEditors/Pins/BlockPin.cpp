#include "BlockPin.h"

ImNodesPinShape BlockPin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(6, 165, 239, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(137, 196, 247, alpha * 255));

	return pinShape;	
}

void BlockPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(6, 165, 239, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(137, 196, 247, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(137, 196, 247, 255));
}

void BlockPin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}

ImU32 BlockPin::editorGetLinkStyleColor() const
{
	return IM_COL32(6, 165, 239, 255);
}