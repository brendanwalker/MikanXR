#pragma once

#include "CompositorConstants.h"
#include "ComponentFwd.h"
#include "Node.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "MikanTypeFwd.h"

class DepthTextureSourceNodeConfig : public NodeConfig
{
public:
	DepthTextureSourceNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eTextureSourceDepthType textureSourceColorType;
	MikanVideoSourceID textureVideoSourceId;
	bool bVerticalFlip;
};

class DepthTextureSourceNode : public Node
{
public:
	DepthTextureSourceNode() = default;

	inline static const std::string k_nodeClassName = "DepthTextureSourceNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	TextureSourceComponentPtr getTextureSourceComponent() const;
	IMkTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	IMkTexturePtr getDepthSourceTexture() const;
	void updateLinearDepthFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture);
	void evaluateDepthTexture(IMkState* glState, IMkTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	IMkFrameBufferPtr m_linearDepthFrameBuffer;
	MkMaterialInstancePtr m_depthMaterialInstance;
	eTextureSourceDepthType m_clientTextureType= eTextureSourceDepthType::depthPackRGBA;
	TextureSourceComponentWeakPtr m_textureSourceComponent;
	bool m_bVerticalFlip= false;
};

class DepthTextureSourceNodeFactory : public TypedNodeFactory<DepthTextureSourceNode, DepthTextureSourceNodeConfig>
{
public:
	DepthTextureSourceNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};