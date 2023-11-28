#pragma once

#include "NodePin.h"

class BlockPin : public NodePin
{
public:
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;
};