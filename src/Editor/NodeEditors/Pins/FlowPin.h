#pragma once

#include "NodePin.h"

class FlowPin : public NodePin
{
public:
	FlowPin();

	inline static const std::string k_pinClassName= "FlowPin";
	virtual std::string getClassName() const override { return k_pinClassName; }
	virtual MkCanvas::PinIcon editorGetPinIcon() const override;
	virtual ImVec4 editorGetPinColor() const override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;
};