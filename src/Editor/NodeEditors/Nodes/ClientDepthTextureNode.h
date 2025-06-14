#pragma once

#include "Node.h"
#include "MikanRendererFwd.h"
#include "FrameCompositorConstants.h"

class ClientDepthTextureNodeConfig : public NodeConfig
{
public:
	ClientDepthTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eClientDepthTextureType clientTextureType;
	std::string clientId;
	bool bVerticalFlip;
};

class ClientDepthTextureNode : public Node
{
public:
	ClientDepthTextureNode() = default;

	inline static const std::string k_nodeClassName = "ClientDepthTextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline const std::string& getClientId() const { return m_clientId; }
	IMkTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	IMkTexturePtr getClientDepthSourceTexture() const;
	void updateLinearDepthFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture);
	void evaluateDepthTexture(IMkState* glState, IMkTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	IMkFrameBufferPtr m_linearDepthFrameBuffer;
	MkMaterialInstancePtr m_depthMaterialInstance;
	eClientDepthTextureType m_clientTextureType= eClientDepthTextureType::depthPackRGBA;
	std::string m_clientId;
	bool m_bVerticalFlip= false;
};

class ClientDepthTextureNodeFactory : public TypedNodeFactory<ClientDepthTextureNode, ClientDepthTextureNodeConfig>
{
public:
	ClientDepthTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};