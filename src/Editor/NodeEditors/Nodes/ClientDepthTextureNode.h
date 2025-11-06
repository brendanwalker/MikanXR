#pragma once

#include "CompositorConstants.h"
#include "ComponentFwd.h"
#include "Node.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"

class ClientDepthTextureNodeConfig : public NodeConfig
{
public:
	ClientDepthTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eClientDepthTextureType clientTextureType;
	MikanVideoSourceID clientVideoSourceId;
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

	ClientVideoSourceComponentPtr getClientVideoSourceComponent() const;
	IMkTexturePtr getTextureResource() const;
	std::string getClientId() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	ClientVideoSourceSystemPtr getClientVideoSourceSystem() const;
	IMkTexturePtr getClientDepthSourceTexture() const;
	void updateLinearDepthFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture);
	void evaluateDepthTexture(IMkState* glState, IMkTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	IMkFrameBufferPtr m_linearDepthFrameBuffer;
	MkMaterialInstancePtr m_depthMaterialInstance;
	eClientDepthTextureType m_clientTextureType= eClientDepthTextureType::depthPackRGBA;
	ClientVideoSourceComponentWeakPtr m_clientVideoSourceComponent;
	bool m_bVerticalFlip= false;
};

class ClientDepthTextureNodeFactory : public TypedNodeFactory<ClientDepthTextureNode, ClientDepthTextureNodeConfig>
{
public:
	ClientDepthTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};