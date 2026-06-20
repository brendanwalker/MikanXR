#pragma once

#include "Node.h"
#include "ComponentFwd.h"

class TextureSourceNodeConfig : public NodeConfig
{
public:
	TextureSourceNodeConfig()
		: NodeConfig()
	{
	}
	TextureSourceNodeConfig(const std::string& nodeName)
		: NodeConfig(nodeName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id textureSourcePropertyId;
};
using TextureSourceNodeConfigPtr= std::shared_ptr<TextureSourceNodeConfig>;
using TextureSourceNodeConfigConstPtr= std::shared_ptr<const TextureSourceNodeConfig>;

class TextureSourceNode : public Node
{
public:
	TextureSourceNode()= default;
	virtual ~TextureSourceNode();

	inline static const std::string k_nodeClassName= "TextureSourceNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphTextureSourcePropertyPtr getTextureSourceProperty() const { return m_sourceProperty; }
	void setTextureSourceProperty(GraphTextureSourcePropertyPtr inTextureSourceProperty);

	TextureSourceComponentPtr getTextureSourceComponent() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::shared_ptr<MkNodesScopedColorStyle> editorRenderMakeNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

	IMkTexturePtr getTextureResource() const;

	void onGraphPropertyDeleted(t_graph_property_id id);

protected:
	GraphTextureSourcePropertyPtr m_sourceProperty;
};

class TextureSourceNodeFactory : public TypedNodeFactory<TextureSourceNode, TextureSourceNodeConfig>
{
public:
	TextureSourceNodeFactory()= default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
	virtual bool editorCanCreate() const override { return false; }
};