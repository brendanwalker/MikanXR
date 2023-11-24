#include "FloatPin.h"

ImNodesPinShape FloatPin::editorRenderBeginPin(float alpha)
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

void FloatPin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}