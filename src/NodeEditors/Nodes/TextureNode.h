#pragma once

#include "Node.h"
#include "RendererFwd.h"

class TextureNode : public Node
{
public:
	TextureNode();
	TextureNode(NodeGraphPtr parentGraph);

	virtual void editorRender(class NodeEditorState* editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(NodeEditorState* editorState) const override;
	virtual std::string editorGetTitle() const override { return "Texture"; }

protected:
	GlTexturePtr m_target;
};