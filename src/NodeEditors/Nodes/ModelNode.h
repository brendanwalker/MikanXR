#pragma once

#include "Node.h"
#include "RendererFwd.h"

class ModelNode : public Node
{
public:
	ModelNode();
	ModelNode(NodeGraphPtr ownerGraph);
	virtual ~ModelNode();

	inline GraphModelPropertyPtr getModelSource() const { return m_sourceProperty; }
	void setModelSource(GraphModelPropertyPtr inModelProperty);

	GlRenderModelResourcePtr getModelResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Model"; }

	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GraphModelPropertyPtr m_sourceProperty;
	GraphVariableListPtr m_modelArrayProperty;
};

class ModelNodeFactory : public NodeFactory
{
public:
	ModelNodeFactory() = default;
	ModelNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};