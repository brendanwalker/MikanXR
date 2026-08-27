#pragma once

#include "Node.h"
#include "ComponentFwd.h"

class ShapeNodeConfig : public NodeConfig
{
public:
	ShapeNodeConfig()
		: NodeConfig()
	{
	}
	ShapeNodeConfig(const std::string& nodeName)
		: NodeConfig(nodeName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id shapePropertyId;
};
using ShapeNodeConfigPtr= std::shared_ptr<ShapeNodeConfig>;
using ShapeNodeConfigConstPtr= std::shared_ptr<const ShapeNodeConfig>;

class ShapeNode : public Node
{
public:
	ShapeNode()= default;
	virtual ~ShapeNode();

	inline static const std::string k_nodeClassName= "ShapeNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphShapePropertyPtr getShapeSource() const { return m_sourceProperty; }
	void setShapeSource(GraphShapePropertyPtr inShapeProperty);

	ShapeComponentPtr getShapeComponent() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;
	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

	void onGraphPropertyDeleted(t_graph_property_id id);

protected:
	GraphShapePropertyPtr m_sourceProperty;
};

class ShapeNodeFactory : public TypedNodeFactory<ShapeNode, ShapeNodeConfig>
{
public:
	ShapeNodeFactory()= default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
	virtual bool editorCanCreate() const override { return false; }
};
