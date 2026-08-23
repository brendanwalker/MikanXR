#pragma once

#include "NodePin.h"

class FlowPin : public NodePin
{
public:
	FlowPin();

	inline static const std::string k_pinClassName= "FlowPin";
	virtual std::string getClassName() const override { return k_pinClassName; }
	virtual ImNodesPinShape editorComputePinShape() const override;
	virtual std::shared_ptr<MkNodesScopedColorStyle> editorRenderMakePinStyle(float alpha) override;
	virtual std::shared_ptr<MkNodesScopedColorStyle> editorRenderMakeLinkStyle(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;
};