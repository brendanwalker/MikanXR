#pragma once

#include "Node.h"
#include "GlTypesFwd.h"

class BlockNode : public Node
{
public:
	BlockNode();
	BlockNode(NodeGraphPtr parentGraph);
	virtual ~BlockNode();

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Block"; }

protected:
	int m_size= 0;
	GLuint m_ubo= -1;
	GLuint m_ssbo= -1;
	int m_ssboSize= 1;
};