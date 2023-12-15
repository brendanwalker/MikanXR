#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"
#include "GraphProperty.h"

class GraphTextureProperty : public GraphProperty
{
public:
	GraphTextureProperty();
	GraphTextureProperty(NodeGraphPtr ownerGraph);

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

class GraphTexturePropertyFactory : public GraphPropertyFactory
{
public:
	GraphTexturePropertyFactory() : GraphPropertyFactory() {}
	GraphTexturePropertyFactory(NodeGraphPtr ownerGraph) : GraphPropertyFactory(ownerGraph) {}

	virtual const std::string getPropertyTypeName() const override { return "texture_property"; }
	virtual GraphPropertyPtr createProperty(
		const class NodeEditorState* editorState,
		const std::string& name) const;
};