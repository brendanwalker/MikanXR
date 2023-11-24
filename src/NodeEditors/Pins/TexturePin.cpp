#include "TexturePin.h"

ImNodesPinShape TexturePin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(148, 0, 0, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(183, 137, 137, alpha * 255));

	return pinShape;	
}

void TexturePin::editorRenderContextMenu(class NodeEditorState* editorState)
{
}