#pragma once

#include "Node.h"
#include "RendererFwd.h"

class MaterialNodeConfig : public NodeConfig
{
public:
	MaterialNodeConfig() : NodeConfig() {}
	MaterialNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id materialPropertyId;
};

class MaterialNode : public Node
{
public:
	MaterialNode() = default;
	virtual ~MaterialNode();

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphMaterialPropertyPtr getMaterialSource() const { return m_sourceProperty; }
	void setMaterialSource(GraphMaterialPropertyPtr inMaterialProperty);

	GlMaterialPtr getMaterialResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Material"; }

	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GraphMaterialPropertyPtr m_sourceProperty;
	GraphVariableListPtr m_materialArrayProperty;
};

class MaterialNodeFactory : public NodeFactory
{
public:
	MaterialNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};