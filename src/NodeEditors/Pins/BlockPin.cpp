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

void BlockPin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}