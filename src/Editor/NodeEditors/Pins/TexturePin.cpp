#include "TexturePin.h"

void TexturePin::copyValueFromSourcePin()
{
	TexturePinPtr sourcePin = std::dynamic_pointer_cast<TexturePin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

ImNodesPinShape TexturePin::editorComputePinShape() const
{
	if (m_connectedLinks.size() > 0)
		return ImNodesPinShape_CircleFilled;
	else
		return ImNodesPinShape_Circle;
}

std::shared_ptr<MkNodesScopedColorStyle> TexturePin::editorRenderMakePinStyle(float alpha)
{
	auto style = std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Pin, IM_COL32(148, 0, 0, (unsigned char)(alpha * 255)))
		.push(ImNodesCol_PinHovered, IM_COL32(183, 137, 137, (unsigned char)(alpha * 255)));
	return style;
}

std::shared_ptr<MkNodesScopedColorStyle> TexturePin::editorRenderMakeLinkStyle(float alpha)
{
	auto style = std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_Link, IM_COL32(148, 0, 0, (unsigned char)alpha))
		.push(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, (unsigned char)alpha))
		.push(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
	return style;
}

void TexturePin::editorRenderContextMenu(const NodeEditorState& editorState)
{
}

ImU32 TexturePin::editorGetLinkStyleColor() const
{
	return IM_COL32(148, 0, 0, 255);
}