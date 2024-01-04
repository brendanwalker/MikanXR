#include "TextureAssetReference.h"
#include "GlTexture.h"

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