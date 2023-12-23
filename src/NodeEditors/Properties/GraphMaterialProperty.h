#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"

class GraphMaterialPropertyConfig : public GraphPropertyConfig
{
public:
	GraphMaterialPropertyConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	int assetRefIndex;
};

class GraphMaterialProperty : public GraphProperty
{
public:
	GraphMaterialProperty()= default;

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	inline void setMaterialAssetReference(MaterialAssetReferencePtr inAssetRef) { m_materialAssetRef = inAssetRef; }
	inline MaterialAssetReferencePtr getMaterialAssetReference() const { return m_materialAssetRef; }

	inline void setMaterialResource(GlMaterialPtr inMaterialResource) { m_materialResource = inMaterialResource; }
	inline GlMaterialPtr getMaterialResource() const { return m_materialResource; }

	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	MaterialAssetReferencePtr m_materialAssetRef;
	GlMaterialPtr m_materialResource;
};

using GraphMaterialPropertyFactory = TypedGraphPropertyFactory<GraphMaterialProperty, GraphMaterialPropertyConfig>;