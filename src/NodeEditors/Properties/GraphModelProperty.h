#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"

class GraphModelPropertyConfig : public GraphPropertyConfig
{
public:
	GraphModelPropertyConfig() : GraphPropertyConfig() {}
	GraphModelPropertyConfig(const std::string& nodeName) : GraphPropertyConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	int assetRefIndex;
};

class GraphModelProperty : public GraphProperty
{
public:
	GraphModelProperty();
	GraphModelProperty(NodeGraphPtr ownerGraph);

	virtual bool loadFromConfig(const class GraphPropertyConfig& config) override;
	virtual void saveToConfig(class GraphPropertyConfig& config) const override;

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

class GraphModelPropertyFactory : public GraphPropertyFactory
{
public:
	GraphModelPropertyFactory() : GraphPropertyFactory() {}
	GraphModelPropertyFactory(NodeGraphPtr ownerGraph) : GraphPropertyFactory(ownerGraph) {}

	virtual GraphPropertyPtr createProperty(
		const class NodeEditorState* editorState,
		const std::string& name) const;
};