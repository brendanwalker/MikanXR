#include "TexturePin.h"

void TexturePin::copyValueFromSourcePin()
{
	TexturePinPtr sourcePin= std::dynamic_pointer_cast<TexturePin>(getConnectedSourcePin());

	if (sourcePin)
	{
		setValue(sourcePin->getValue());
	}
}

MkCanvas::PinIcon TexturePin::editorGetPinIcon() const { return MkCanvas::PinIcon::Circle; }

ImVec4 TexturePin::editorGetPinColor() const { return ImVec4(148.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1.f); }

void TexturePin::editorRenderContextMenu(const NodeEditorState& editorState) {}

ImU32 TexturePin::editorGetLinkStyleColor() const { return IM_COL32(148, 0, 0, 255); }