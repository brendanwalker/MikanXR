#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"

class GraphMaterialProperty : public GraphProperty
{
public:
	GraphMaterialProperty();
	GraphMaterialProperty(NodeGraphPtr ownerGraph);

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

class GraphMaterialPropertyFactory : public GraphPropertyFactory
{
public:
	GraphMaterialPropertyFactory() : GraphPropertyFactory() {}
	GraphMaterialPropertyFactory(NodeGraphPtr ownerGraph) : GraphPropertyFactory(ownerGraph) {}

	virtual const std::string getPropertyTypeName() const override { return "material_property"; }
	virtual GraphPropertyPtr createProperty(
		const class NodeEditorState* editorState,
		const std::string& name) const;
};