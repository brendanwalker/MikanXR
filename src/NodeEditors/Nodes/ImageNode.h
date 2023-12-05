#pragma once

#include "Node.h"
#include "GlTypesFwd.h"

class ImageNode : public Node
{
public:
	ImageNode();
	ImageNode(NodeGraphPtr parentGraph);
	virtual ~ImageNode();

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Image"; }

protected:
	int m_sizeX= 0;
	int m_sizeY= 0;
	GLuint m_texture= -1;
};