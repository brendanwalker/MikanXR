#pragma once

#include "Node.h"
#include "RendererFwd.h"

class TextureNodeConfig : public NodeConfig
{
public:
	TextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id texturePropertyId;
};

class TextureNode : public Node
{
public:
	TextureNode() = default;
	virtual ~TextureNode();

	inline static const std::string k_nodeClassName = "TextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphTexturePropertyPtr getTextureSource() const { return m_sourceProperty; }
	void setTextureSource(GraphTexturePropertyPtr inTextureProperty);

	GlTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Texture"; }

	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GraphTexturePropertyPtr m_sourceProperty;
	GraphVariableListPtr m_textureArrayProperty;
};

class TextureNodeFactory : public TypedNodeFactory<TextureNode, TextureNodeConfig>
{
public:
	TextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};