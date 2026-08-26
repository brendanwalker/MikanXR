#pragma once

#include "LocText.h"
#include "Node.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include "IVideoDevice.h"

class VideoTextureNodeConfig : public NodeConfig
{
public:
	VideoTextureNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eVideoTextureSource videoTextureSource;
	eVideoTransferFunction transferFunctionOverride= eVideoTransferFunction::INVALID;
};

class VideoTextureNode : public Node
{
public:
	VideoTextureNode()= default;

	inline static const std::string k_nodeClassName= "VideoTextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::shared_ptr<MkNodesScopedColorStyle> editorRenderMakeNodeStyle(
		const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return locText("nodes.videoTextureTitle"); }

	IMkTexturePtr getTextureResource() const;
	IMkTexturePtr getPreviewTextureResource() const;

	// Renders the raw (non-linear) video texture through the linearize material into
	// m_linearFrameBuffer and returns the resulting linear-color-space texture.
	IMkTexturePtr linearizeVideoTexture(NodeEvaluator& evaluator, IMkTexturePtr sourceTexture);
	eVideoTransferFunction getColorTransferFunction() const;

protected:
	eVideoTextureSource m_videoTextureSource= eVideoTextureSource::video_texture;
	eVideoTransferFunction m_transferFunctionOverride= eVideoTransferFunction::INVALID;

	IMkFrameBufferPtr m_linearFrameBuffer;
	MkMaterialInstancePtr m_linearizeMaterialInstance;
};

class VideoTextureNodeFactory : public TypedNodeFactory<VideoTextureNode, VideoTextureNodeConfig>
{
public:
	VideoTextureNodeFactory()= default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};