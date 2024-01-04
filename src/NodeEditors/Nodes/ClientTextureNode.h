#pragma once

#include "Node.h"
#include "RendererFwd.h"
#include "FrameCompositorConstants.h"

class ClientTextureNodeConfig : public NodeConfig
{
public:
	ClientTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eClientTextureType clientTextureType;
	int clientIndex;
};

class ClientTextureNode : public Node
{
public:
	ClientTextureNode() = default;

	inline static const std::string k_nodeClassName = "ClientTextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	GlTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Client Texture"; }

protected:
	eClientTextureType m_clientTextureType= eClientTextureType::color;
	int m_clientIndex= 0;
};

class ClientTextureNodeFactory : public TypedNodeFactory<ClientTextureNode, ClientTextureNodeConfig>
{
public:
	ClientTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};