#pragma once

#include "Node.h"
#include "RendererFwd.h"

class ProgramNode : public Node
{
public:
	ProgramNode();
	ProgramNode(NodeGraphPtr parentGraph);
	virtual ~ProgramNode();

	void setFramebuffer(GlFrameBufferPtr inFrameBuffer);

	virtual bool evaluateNode(NodeEvaluator& evaluator);
	virtual FlowPinPtr getOutputFlowPin() const;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::string editorGetTitle() const override;
	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GlProgramPtr m_target;
	FrameBufferArrayPropertyPtr m_frameBufferArrayProperty;
	int m_attachmentsPinsStartId;
	GlFrameBufferPtr m_framebuffer;
	NodePinPtr m_flowIn;
	NodePinPtr m_flowOut;
	GLenum m_drawMode;
	int m_dispatchSize;
};

class ProgramNodeFactory : public NodeFactory
{
public:
	ProgramNodeFactory() = default;
	ProgramNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};