#pragma once

#include "NodePin.h"

class TexturePin : public NodePin
{
public:
	TexturePin() : NodePin() {}
	TexturePin(NodePtr ownerNode) : NodePin(ownerNode) {}

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;
};