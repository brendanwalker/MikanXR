#pragma once

#include "RendererFwd.h"
#include "NodePin.h"

class TexturePin : public NodePin
{
public:
	TexturePin() : NodePin() {}
	TexturePin(NodePtr ownerNode) : NodePin(ownerNode) {}

	GlTexturePtr getValue() const { return m_value; }
	void setValue(GlTexturePtr inValue) { m_value = inValue; }

	virtual size_t getDataSize() const { return sizeof(GlTexturePtr); }
	virtual void copyValueFromSourcePin() override;

	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	GlTexturePtr m_value;
};