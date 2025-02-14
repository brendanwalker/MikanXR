#pragma once

#include "Node.h"
#include "ComponentFwd.h"

class StencilNodeConfig : public NodeConfig
{
public:
	StencilNodeConfig() : NodeConfig() {}
	StencilNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id stencilPropertyId;
};
using StencilNodeConfigPtr = std::shared_ptr<StencilNodeConfig>;
using StencilNodeConfigConstPtr = std::shared_ptr<const StencilNodeConfig>;

class StencilNode : public Node
{
public:
	StencilNode() = default;
	virtual ~StencilNode();

	inline static const std::string k_nodeClassName = "StencilNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphStencilPropertyPtr getStencilSource() const { return m_sourceProperty; }
	void setStencilSource(GraphStencilPropertyPtr inStencilProperty);

	StencilComponentPtr getStencilComponent() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

	void onGraphPropertyDeleted(t_graph_property_id id);

protected:
	GraphStencilPropertyPtr m_sourceProperty;
};

class StencilNodeFactory : public TypedNodeFactory<StencilNode, StencilNodeConfig>
{
public:
	StencilNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
	virtual bool editorCanCreate() const override { return false; }
};