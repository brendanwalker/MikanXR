#include "IntPin.h"

ImNodesPinShape IntPin::editorRenderBeginPin(float alpha)
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

void IntPin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}