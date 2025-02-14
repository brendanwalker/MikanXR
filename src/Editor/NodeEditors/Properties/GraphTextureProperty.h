#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
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

	inline static const std::string k_propertyClassName = "GraphTextureProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	void setTextureAssetReference(TextureAssetReferencePtr inAssetRef);
	inline TextureAssetReferencePtr getTextureAssetReference() const { return m_textureAssetRef; }

	inline IMkTexturePtr getTextureResource() const { return m_texture; }

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual std::string editorGetTitle() const override { return "Texture"; }
	virtual const ImVec4 editorGetIconColor() const override;

protected:
	TextureAssetReferencePtr m_textureAssetRef;
	IMkTexturePtr m_texture;
};

using GraphTexturePropertyFactory = TypedGraphPropertyFactory<GraphTextureProperty, GraphTexturePropertyConfig>;