#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"

class GraphModelProperty : public GraphProperty
{
public:
	GraphModelProperty();
	GraphModelProperty(NodeGraphPtr ownerGraph);

	inline void setModelAssetReference(ModelAssetReferencePtr inAssetRef) { m_modelAssetRef= inAssetRef; }
	inline ModelAssetReferencePtr getModelAssetReference() const { return m_modelAssetRef; }

	inline void setModelResource(GlRenderModelResourcePtr inModelResource) { m_modelResource= inModelResource; }
	inline GlRenderModelResourcePtr getModelResource() const { return m_modelResource; }

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

	virtual const std::string getPropertyTypeName() const override { return "model_property"; }
	virtual GraphPropertyPtr createProperty(
		const class NodeEditorState* editorState,
		const std::string& name) const;
};