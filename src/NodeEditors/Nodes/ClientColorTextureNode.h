#pragma once

#include "Node.h"
#include "RendererFwd.h"
#include "FrameCompositorConstants.h"

class ClientColorTextureNodeConfig : public NodeConfig
{
public:
	ClientColorTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eClientColorTextureType clientTextureType;
	int clientIndex;
	bool bVerticalFlip;
};

class ClientColorTextureNode : public Node
{
public:
	ClientColorTextureNode() = default;

	inline static const std::string k_nodeClassName = "ClientColorTextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	GlTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	GlTexturePtr getClientColorSourceTexture() const;
	void updateColorFrameBuffer(NodeEvaluator& evaluator, GlTexturePtr clientTexture);
	void evaluateFlippedColorTexture(GlState& glState, GlTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	GlFrameBufferPtr m_colorFrameBuffer;
	GlMaterialInstancePtr m_colorMaterialInstance;
	eClientColorTextureType m_clientTextureType= eClientColorTextureType::colorRGB;
	int m_clientIndex= 0;
	bool m_bVerticalFlip= false;
};

class ClientColorTextureNodeFactory : public TypedNodeFactory<ClientColorTextureNode, ClientColorTextureNodeConfig>
{
public:
	ClientColorTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};