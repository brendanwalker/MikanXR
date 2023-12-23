#pragma once

#include "Node.h"
#include "RendererFwd.h"

class ModelNodeConfig : public NodeConfig
{
public:
	ModelNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id modelPropertyId;
};

class ModelNode : public Node
{
public:
	ModelNode() = default;
	virtual ~ModelNode();

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

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

class ModelNodeFactory : public TypedNodeFactory<ModelNode, ModelNodeConfig>
{
public:
	ModelNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};