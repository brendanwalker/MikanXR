#include "ImagePin.h"

ImNodesPinShape ImagePin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(200, 130, 255, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(220, 170, 255, alpha * 255));

	return pinShape;	
}

void ImagePin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}