#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
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

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfig& config) const override;

	inline void setModelAssetReference(ModelAssetReferencePtr inAssetRef) { m_modelAssetRef= inAssetRef; }
	inline ModelAssetReferencePtr getModelAssetReference() const { return m_modelAssetRef; }

	inline void setModelResource(GlRenderModelResourcePtr inModelResource) { m_modelResource= inModelResource; }
	inline GlRenderModelResourcePtr getModelResource() const { return m_modelResource; }

	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	ModelAssetReferencePtr m_modelAssetRef;
	GlRenderModelResourcePtr m_modelResource;
};

using GraphModelPropertyFactory = TypedGraphPropertyFactory<GraphModelProperty, GraphModelPropertyConfig>;