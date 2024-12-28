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

	inline static const std::string k_propertyClassName = "GraphMaterialProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	void setMaterialAssetReference(MaterialAssetReferencePtr inAssetRef);
	inline MaterialAssetReferencePtr getMaterialAssetReference() const { return m_materialAssetRef; }

	inline GlMaterialConstPtr getMaterialResource() const { return m_materialResource; }

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual std::string editorGetTitle() const override { return "Material"; }

protected:
	MaterialAssetReferencePtr m_materialAssetRef;
	GlMaterialConstPtr m_materialResource;
};

using GraphMaterialPropertyFactory = TypedGraphPropertyFactory<GraphMaterialProperty, GraphMaterialPropertyConfig>;