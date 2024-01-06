#include "TextureAssetReference.h"
#include "GlTexture.h"
#include "PathUtils.h"

// -- MaterialAssetReference -----
void TextureAssetReference::rebuildPreview()
{
	if (!m_previewTexture)
	{
		m_previewTexture = std::make_shared<GlTexture>();
	}

	m_previewTexture->setImagePath(getAssetPath());
	m_previewTexture->reloadTextureFromImagePath();
}

void TextureAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// TODO: Render material asset properties 
}

// -- TextureAssetReferenceFactory -----
TextureAssetReferenceFactory::TextureAssetReferenceFactory()
	: TypedAssetReferenceFactory<TextureAssetReference, AssetReferenceConfig>()
{
	m_defaultPath = (PathUtils::getResourceDirectory() / "textures" / "").string();
}