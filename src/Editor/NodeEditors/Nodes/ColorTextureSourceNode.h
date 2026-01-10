#pragma once

#include "CompositorConstants.h"
#include "ComponentFwd.h"
#include "Node.h"
#include "MikanTypeFwd.h"
#include "MikanRendererFwd.h"

class ColorTextureSourceNodeConfig : public NodeConfig
{
public:
	ColorTextureSourceNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eTextureSourceColorType textureSourceColorType;
	MikanTextureSourceID textureSourceId;
	bool bVerticalFlip;
};

class ColorTextureSourceNode : public Node
{
public:
	ColorTextureSourceNode() = default;

	inline static const std::string k_nodeClassName = "ColorTextureSourceNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	TextureSourceComponentPtr getTextureSourceComponent() const;
	IMkTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	IMkTexturePtr getColorSourceTexture() const;
	void updateColorFrameBuffer(NodeEvaluator& evaluator, IMkTexturePtr clientTexture);
	void evaluateFlippedColorTexture(IMkState* glState, IMkTexturePtr depthTexture);

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	IMkFrameBufferPtr m_colorFrameBuffer;
	MkMaterialInstancePtr m_colorMaterialInstance;
	eTextureSourceColorType m_clientTextureType= eTextureSourceColorType::colorRGB;
	TextureSourceComponentWeakPtr m_textureSourceComponent;
	bool m_bVerticalFlip= false;
};

class ColorTextureSourceNodeFactory : public TypedNodeFactory<ColorTextureSourceNode, ColorTextureSourceNodeConfig>
{
public:
	ColorTextureSourceNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};