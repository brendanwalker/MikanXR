#include "TextureAssetReference.h"
#include "IMkTexture.h"
#include "NodeEditorState.h"
#include "PathUtils.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"

#include "Graphs/NodeGraph.h"
#include "Nodes/TextureNode.h"
#include "Properties/GraphTextureProperty.h"

// -- MaterialAssetReference -----
void TextureAssetReference::rebuildPreview()
{
	if (!m_previewTexture)
	{
		m_previewTexture= CreateMkTexture();
	}

	m_previewTexture->setImagePath(getResolvedAssetPath().string());
	m_previewTexture->reloadTextureFromImagePath();
}

void TextureAssetReference::editorHandleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	auto self= std::static_pointer_cast<TextureAssetReference>(shared_from_this());

	// Create a texture property to hold the reference to this asset
	auto textureProperty= editorState.nodeGraph->createTypedProperty<GraphTextureProperty>();
	textureProperty->setTextureAssetReference(self);
	textureProperty->setName(editorState.nodeGraph->makeUniquePropertyName(getShortName()));
}

void TextureAssetReference::editorHandleMainFrameDragDrop(const NodeEditorState& editorState)
{
	auto self= std::static_pointer_cast<TextureAssetReference>(shared_from_this());

	// Create an material property first to hold the reference to this asset
	auto textureProperty= editorState.nodeGraph->createTypedProperty<GraphTextureProperty>();
	textureProperty->setTextureAssetReference(self);
	textureProperty->setName(editorState.nodeGraph->makeUniquePropertyName(getShortName()));

	// Then create a material node in the graph that references the material property
	auto textureNode= editorState.nodeGraph->createTypedNode<TextureNode>(editorState);
	textureNode->setTextureSource(textureProperty);
}

void TextureAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
								   locLabel("assets.textureAssetHeader"));
}

// -- TextureAssetReferenceFactory -----
TextureAssetReferenceFactory::TextureAssetReferenceFactory()
	: TypedAssetReferenceFactory<TextureAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= getDefaultTexturePath();
}

std::string TextureAssetReferenceFactory::getDefaultTexturePath()
{
	return (PathUtils::getProjectDirectory() / "textures" / "").string();
}