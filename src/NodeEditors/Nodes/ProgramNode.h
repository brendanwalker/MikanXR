#pragma once

#include "Node.h"
#include "RendererFwd.h"

enum class ProgramDispatchType : int
{
	ARRAY,
	COMPUTE
};

class ProgramNode : public Node
{
public:
	ProgramNode();
	ProgramNode(NodeGraphPtr parentGraph);

protected:
	virtual std::string editorGetTitle() const override;

protected:
	GlProgramPtr m_target;
	GlFrameBufferPtr m_framebuffer;
	int m_attachmentsPinsStartId;
	NodePinPtr m_flowIn;
	NodePinPtr m_flowOut;
	ProgramDispatchType m_dispatchType;
	GLenum m_drawMode;
	int m_dispatchSize[3];
};