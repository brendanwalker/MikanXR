#include "StencilPin.h"

void StencilPin::copyValueFromSourcePin()
{
	StencilPinPtr sourcePin = std::dynamic_pointer_cast<StencilPin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

ImNodesPinShape StencilPin::editorRenderBeginPin(float alpha)
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

void StencilPin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(148, 0, 0, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
}

void StencilPin::editorRenderContextMenu(const NodeEditorState& editorState)
{}

ImU32 StencilPin::editorGetLinkStyleColor() const
{
	return IM_COL32(148, 0, 0, 255);
}