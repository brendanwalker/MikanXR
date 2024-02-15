#include "MaterialAssetReference.h"
#include "NodeEditorState.h"
#include "PathUtils.h"
#include "NodeEditorUI.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"
#include "Nodes/MaterialNode.h"
#include "Properties/GraphMaterialProperty.h"

#include "IconsForkAwesome.h"
#include "tinyfiledialogs.h"

// -- MaterialAssetReference -----
void MaterialAssetReference::rebuildPreview()
{
	//TODO
}

void MaterialAssetReference::editorHandleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	auto self = std::static_pointer_cast<MaterialAssetReference>(shared_from_this());

	// Create an material property to hold the reference to this asset
	auto materialProperty = editorState.nodeGraph->createTypedProperty<GraphMaterialProperty>();
	materialProperty->setMaterialAssetReference(self);
}

void MaterialAssetReference::editorHandleMainFrameDragDrop(const NodeEditorState& editorState)
{	
	auto self = std::static_pointer_cast<MaterialAssetReference>(shared_from_this());

	// Create an material property first to hold the reference to this asset
	auto materialProperty= editorState.nodeGraph->createTypedProperty<GraphMaterialProperty>();
	materialProperty->setMaterialAssetReference(self);

	// Then create a material node in the graph that references the material property
	auto materialNode = editorState.nodeGraph->createTypedNode<MaterialNode>(editorState);
	materialNode->setMaterialSource(materialProperty);
}

void MaterialAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Material Asset"))
	{
		const std::string buttonName = StringUtils::stringify(ICON_FK_FOLDER_OPEN, "Material##material");

		if (ImGui::SmallButton(buttonName.c_str()))
		{
			static std::string materialPath= MaterialAssetReferenceFactory::getDefaultMaterialPath();
			static const char* filterItems[1] = {"*.mat"};

			auto assetPath =
				tinyfd_openFileDialog(
					"Load Material",
					materialPath.c_str(),
					1,
					filterItems,
					"Material Files (*.mat)",
					0); // disallow multiple selections
			setAssetPath(assetPath);
		}
	}
}

// -- MaterialAssetReferenceFactory -----
MaterialAssetReferenceFactory::MaterialAssetReferenceFactory()
	: TypedAssetReferenceFactory<MaterialAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= getDefaultMaterialPath();
}

std::string MaterialAssetReferenceFactory::getDefaultMaterialPath()
{
	return (PathUtils::getResourceDirectory() / "shaders" / "compositor" / "").string();
}