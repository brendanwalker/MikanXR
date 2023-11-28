#pragma once

#include "Node.h"
#include "GlTypesFwd.h"

class BlockNode : public Node
{
public:
	BlockNode();
	BlockNode(NodeGraphPtr parentGraph);
	virtual ~BlockNode();

	virtual void editorRender(class NodeEditorState* editorState) override;

public:
	int m_size= 0;
	GLuint m_ubo= -1;
	GLuint m_ssbo= -1;
	int m_ssboSize= 1;
};