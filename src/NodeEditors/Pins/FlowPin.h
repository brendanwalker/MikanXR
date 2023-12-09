#pragma once

#include "NodePin.h"

class FlowPin : public NodePin
{
public:
	FlowPin() : NodePin() {}
	FlowPin(NodePtr ownerNode) : NodePin(ownerNode) {}

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;
};