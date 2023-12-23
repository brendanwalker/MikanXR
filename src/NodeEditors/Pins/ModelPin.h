#pragma once

#include "RendererFwd.h"
#include "NodePin.h"

class ModelPin : public NodePin
{
public:
	ModelPin() = default;

	GlRenderModelResourcePtr getValue() const { return m_value; }
	void setValue(GlRenderModelResourcePtr inValue) { m_value = inValue; }

	virtual size_t getDataSize() const { return sizeof(GlRenderModelResourcePtr); }
	virtual void copyValueFromSourcePin() override;

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	GlRenderModelResourcePtr m_value;
};