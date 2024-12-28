#pragma once

#include "Node.h"
#include "RendererFwd.h"
#include "VideoDisplayConstants.h"

class VideoTextureNodeConfig : public NodeConfig
{
public:
	VideoTextureNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eVideoTextureSource videoTextureSource;
};

class VideoTextureNode : public Node
{
public:
	VideoTextureNode() = default;

	inline static const std::string k_nodeClassName = "VideoTextureNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	GlTexturePtr getTextureResource() const;
	GlTexturePtr getPreviewTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Video Texture"; }

protected:
	eVideoTextureSource m_videoTextureSource= eVideoTextureSource::video_texture;
};

class VideoTextureNodeFactory : public TypedNodeFactory<VideoTextureNode, VideoTextureNodeConfig>
{
public:
	VideoTextureNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};