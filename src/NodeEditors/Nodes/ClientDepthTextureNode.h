#pragma once

#include "Node.h"
#include "RendererFwd.h"
#include "FrameCompositorConstants.h"

class ClientDepthTextureNodeConfig : public NodeConfig
{
public:
	ClientDepthTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eClientDepthTextureType clientTextureType;
	int clientIndex;
};

class ClientDepthTextureNode : public Node
{
public:
	ClientDepthTextureNode() = default;

	inline static const std::string k_nodeClassName = "ClientDepthTextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	GlTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	GlTexturePtr getClientDepthSourceTexture() const;
	void updateLinearDepthFrameBuffer(NodeEvaluator& evaluator, GlTexturePtr clientTexture);
	void evaluateDepthTexture(GlState& glState, GlTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	GlFrameBufferPtr m_linearDepthFrameBuffer;
	GlMaterialInstancePtr m_depthMaterialInstance;
	eClientDepthTextureType m_clientTextureType= eClientDepthTextureType::depthPackRGBA;
	int m_clientIndex= 0;
};

class ClientDepthTextureNodeFactory : public TypedNodeFactory<ClientDepthTextureNode, ClientDepthTextureNodeConfig>
{
public:
	ClientDepthTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};