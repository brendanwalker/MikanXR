#pragma once

#include "Node.h"
#include "MikanRendererFwd.h"

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

	inline static const std::string k_nodeClassName = "MaterialNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphMaterialPropertyPtr getMaterialSource() const { return m_sourceProperty; }
	void setMaterialSource(GraphMaterialPropertyPtr inMaterialProperty);

	GlMaterialConstPtr getMaterialResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState);

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

	void onGraphPropertyDeleted(t_graph_property_id id);

protected:
	GraphMaterialPropertyPtr m_sourceProperty;
};

class MaterialNodeFactory : public TypedNodeFactory<MaterialNode, MaterialNodeConfig>
{
public:
	MaterialNodeFactory() = default;

	virtual bool editorCanCreate() const override { return false; }
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};