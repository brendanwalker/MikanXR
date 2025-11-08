#pragma once

#include "CompositorConstants.h"
#include "ComponentFwd.h"
#include "Node.h"
#include "MikanTypeFwd.h"
#include "MikanRendererFwd.h"

class ClientColorTextureNodeConfig : public NodeConfig
{
public:
	ClientColorTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eTextureSourceColorType clientTextureType;
	MikanTextureSourceID clientTextureSourceId;
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

	ClientTextureSourceComponentPtr getClientTextureSourceComponent() const;
	IMkTexturePtr getTextureResource() const;
	std::string getClientId() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	ClientTextureSourceSystemPtr getClientTextureSourceSystem() const;
	IMkTexturePtr getClientColorSourceTexture() const;
	void updateColorFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture);
	void evaluateFlippedColorTexture(IMkState* glState, IMkTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	IMkFrameBufferPtr m_colorFrameBuffer;
	MkMaterialInstancePtr m_colorMaterialInstance;
	eTextureSourceColorType m_clientTextureType= eTextureSourceColorType::colorRGB;
	ClientTextureSourceComponentWeakPtr m_clientTextureSourceComponent;
	bool m_bVerticalFlip= false;
};

class ClientColorTextureNodeFactory : public TypedNodeFactory<ClientColorTextureNode, ClientColorTextureNodeConfig>
{
public:
	ClientColorTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};