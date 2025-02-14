#include "TextureAssetReference.h"
#include "IMkTexture.h"
#include "NodeEditorState.h"
#include "PathUtils.h"
#include "NodeEditorUI.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"
#include "Nodes/TextureNode.h"
#include "Properties/GraphTextureProperty.h"

#include "IconsForkAwesome.h"
#include "tinyfiledialogs.h"

// -- MaterialAssetReference -----
void TextureAssetReference::rebuildPreview()
{
	if (!m_previewTexture)
	{
		m_previewTexture = CreateMkTexture();
	}

	m_previewTexture->setImagePath(getAssetPath());
	m_previewTexture->reloadTextureFromImagePath();
}

void TextureAssetReference::editorHandleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	auto self = std::static_pointer_cast<TextureAssetReference>(shared_from_this());

	// Create a texture property to hold the reference to this asset
	auto textureProperty = editorState.nodeGraph->createTypedProperty<GraphTextureProperty>();
	textureProperty->setTextureAssetReference(self);
}

void TextureAssetReference::editorHandleMainFrameDragDrop(const NodeEditorState& editorState)
{
	auto self = std::static_pointer_cast<TextureAssetReference>(shared_from_this());

	// Create an material property first to hold the reference to this asset
	auto textureProperty = editorState.nodeGraph->createTypedProperty<GraphTextureProperty>();
	textureProperty->setTextureAssetReference(self);

	// Then create a material node in the graph that references the material property
	auto textureNode = editorState.nodeGraph->createTypedNode<TextureNode>(editorState);
	textureNode->setTextureSource(textureProperty);
}

void TextureAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Texture Asset"))
	{
		const std::string buttonName = StringUtils::stringify(ICON_FK_FOLDER_OPEN, "Texture##texture");

		if (ImGui::SmallButton(buttonName.c_str()))
		{
			static std::string texturePath = TextureAssetReferenceFactory::getDefaultTexturePath();

			auto assetPath =
				tinyfd_openFileDialog(
					"Load Texture",
					texturePath.c_str(),
					TextureAssetReferenceFactory::getTextureFilterPatternCount(),
					TextureAssetReferenceFactory::getTextureFilterPatterns(),
					TextureAssetReferenceFactory::getTextureFilterDescription(),
					0); // disallow multiple selections
			setAssetPath(assetPath);
		}
	}
}

// -- TextureAssetReferenceFactory -----
TextureAssetReferenceFactory::TextureAssetReferenceFactory()
	: TypedAssetReferenceFactory<TextureAssetReference, AssetReferenceConfig>()
{
	m_defaultPath = getDefaultTexturePath();
}

std::string TextureAssetReferenceFactory::getDefaultTexturePath()
{
	return (PathUtils::getResourceDirectory() / "textures" / "").string();
}