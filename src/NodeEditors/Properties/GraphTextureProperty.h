#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"

class GraphTexturePropertyConfig : public GraphPropertyConfig
{
public:
	GraphTexturePropertyConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	int assetRefIndex;
};

class GraphTextureProperty : public GraphProperty
{
public:
	GraphTextureProperty() = default;

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfig& config) const override;

	inline void setTextureAssetReference(TextureAssetReferencePtr inAssetRef) { m_textureAssetRef = inAssetRef; }
	inline TextureAssetReferencePtr getTextureAssetReference() const { return m_textureAssetRef; }

	inline void setTextureResource(GlTexturePtr inTexture) { m_texture = inTexture; }
	inline GlTexturePtr getTextureResource() const { return m_texture; }

	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	TextureAssetReferencePtr m_textureAssetRef;
	GlTexturePtr m_texture;
};

using GraphTexturePropertyFactory = TypedGraphPropertyFactory<GraphTextureProperty, GraphTexturePropertyConfig>;