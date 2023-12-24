#pragma once

#include "RendererFwd.h"
#include "NodePin.h"

class MaterialPin : public NodePin
{
public:
	MaterialPin() = default;

	GlMaterialPtr getValue() const { return m_value; }
	void setValue(GlMaterialPtr inValue) { m_value = inValue; }

	virtual std::string getClassName() const override { return "MaterialPin"; }
	virtual size_t getDataSize() const { return sizeof(GlMaterialPtr); }
	virtual void copyValueFromSourcePin() override;

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	GlMaterialPtr m_value;
};