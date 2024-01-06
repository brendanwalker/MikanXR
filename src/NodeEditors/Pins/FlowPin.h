#pragma once

#include "NodePin.h"

class FlowPin : public NodePin
{
public:
	FlowPin();

	virtual std::string getClassName() const override { return "FlowPin"; }
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;
};