#pragma once

#include "Node.h"
#include "GlTypesFwd.h"

class ImageNode : public Node
{
public:
	ImageNode();
	ImageNode(NodeGraphPtr parentGraph);
	virtual ~ImageNode();

	virtual void editorRender(class NodeEditorState* editorState) override;

public:
	int m_sizeX= 0;
	int m_sizeY= 0;
	GLuint m_texture= -1;
};