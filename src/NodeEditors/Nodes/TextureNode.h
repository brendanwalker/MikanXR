#pragma once

#include "Node.h"
#include "RendererFwd.h"

class TextureNode : public Node
{
public:
	TextureNode();
	TextureNode(NodeGraphPtr parentGraph);

	virtual void editorRender(class NodeEditorState* editorState) override;

public:
	GlTexturePtr m_target;
};