#pragma once

#include "Node.h"
#include "MikanRendererFwd.h"

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

	inline static const std::string k_nodeClassName = "ModelNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphModelPropertyPtr getModelSource() const { return m_sourceProperty; }
	void setModelSource(GraphModelPropertyPtr inModelProperty);

	MikanRenderModelResourcePtr getModelResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

	void onGraphPropertyDeleted(t_graph_property_id id);

protected:
	GraphModelPropertyPtr m_sourceProperty;
};

class ModelNodeFactory : public TypedNodeFactory<ModelNode, ModelNodeConfig>
{
public:
	ModelNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
	virtual bool editorCanCreate() const override { return false; }
};