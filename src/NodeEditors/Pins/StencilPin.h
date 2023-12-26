#pragma once

#include "ComponentFwd.h"
#include "NodePin.h"

class StencilPin : public NodePin
{
public:
	StencilPin() = default;

	StencilComponentPtr getValue() const { return m_value; }
	void setValue(StencilComponentPtr inValue) { m_value = inValue; }

	virtual std::string getClassName() const override { return "StencilPin"; }
	virtual size_t getDataSize() const { return sizeof(StencilComponentPtr); }
	virtual void copyValueFromSourcePin() override;

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	StencilComponentPtr m_value;
};