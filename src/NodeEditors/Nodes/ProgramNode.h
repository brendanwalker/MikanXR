#pragma once

#include "Node.h"
#include "RendererFwd.h"

enum class eProgramDispatchType : int
{
	ARRAY,
	COMPUTE
};

class ProgramNode : public Node
{
public:
	ProgramNode();
	ProgramNode(NodeGraphPtr parentGraph);
	virtual ~ProgramNode();

	void setFramebuffer(GlFrameBufferPtr inFrameBuffer);

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::string editorGetTitle() const override;
	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GlProgramPtr m_target;
	FrameBufferArrayPropertyPtr m_frameBufferArrayProperty;
	GlFrameBufferPtr m_framebuffer;
	int m_attachmentsPinsStartId;
	NodePinPtr m_flowIn;
	NodePinPtr m_flowOut;
	eProgramDispatchType m_dispatchType;
	GLenum m_drawMode;
	int m_dispatchSize[3];
};