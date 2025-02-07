#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "GraphProperty.h"

class GraphModelPropertyConfig : public GraphPropertyConfig
{
public:
	GraphModelPropertyConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	int assetRefIndex;
};

class GraphModelProperty : public GraphProperty
{
public:
	GraphModelProperty() = default;

	inline static const std::string k_propertyClassName = "GraphModelProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	inline void setModelAssetReference(ModelAssetReferencePtr inAssetRef) { m_modelAssetRef= inAssetRef; }
	inline ModelAssetReferencePtr getModelAssetReference() const { return m_modelAssetRef; }

	inline void setModelResource(GlRenderModelResourcePtr inModelResource) { m_modelResource= inModelResource; }
	inline GlRenderModelResourcePtr getModelResource() const { return m_modelResource; }

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual std::string editorGetTitle() const override { return "Model"; }

protected:
	ModelAssetReferencePtr m_modelAssetRef;
	GlRenderModelResourcePtr m_modelResource;
};

using GraphModelPropertyFactory = TypedGraphPropertyFactory<GraphModelProperty, GraphModelPropertyConfig>;