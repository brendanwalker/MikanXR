#include "MaterialAssetReference.h"
#include "App.h"
#include "Logger.h"
#include "MikanShaderConfig.h"
#include "NodeEditorState.h"
#include "PathUtils.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"
#include "Nodes/MaterialNode.h"
#include "Properties/GraphMaterialProperty.h"
#include "Windows/MaterialNodeEditorWindow.h"

#include "IconsForkAwesome.h"
#include "tinyfiledialogs.h"

// -- MaterialAssetReference -----
void MaterialAssetReference::rebuildPreview()
{
	// TODO
}

void MaterialAssetReference::setAssetPath(const std::filesystem::path& inPath)
{
	// The source graph belongs to the .mat, so a new path reads it again
	m_bSourceGraphPathResolved= false;
	AssetReference::setAssetPath(inPath);
}

const std::filesystem::path& MaterialAssetReference::getSourceGraphPath() const
{
	if (!m_bSourceGraphPathResolved)
	{
		m_sourceGraphPath.clear();

		if (!m_assetPath.empty())
		{
			MikanShaderConfig materialConfig;
			if (materialConfig.load(m_assetPath) && !materialConfig.sourceGraphPath.empty())
			{
				// Relative to the .mat folder, like the shader paths
				const std::filesystem::path materialFolder= materialConfig.getLoadedConfigPath().parent_path();
				m_sourceGraphPath= (materialFolder / materialConfig.sourceGraphPath).lexically_normal();
			}
		}

		m_bSourceGraphPathResolved= true;
	}

	return m_sourceGraphPath;
}

bool MaterialAssetReference::editorCanOpen() const { return !getSourceGraphPath().empty(); }

void MaterialAssetReference::editorOpen()
{
	const std::filesystem::path& graphPath= getSourceGraphPath();
	if (graphPath.empty())
	{
		MIKAN_LOG_INFO("MaterialAssetReference::editorOpen") << "Material has no source graph: " << m_assetPath;
		return;
	}

	// One material editor window serves every material, so reuse the open one
	App* app= App::getInstance();
	MaterialNodeEditorWindow* materialWindow= app->getWindowOfType<MaterialNodeEditorWindow>();
	if (materialWindow == nullptr)
	{
		materialWindow= app->createAppWindow<MaterialNodeEditorWindow>();
	}

	if (materialWindow == nullptr)
	{
		MIKAN_LOG_ERROR("MaterialAssetReference::editorOpen") << "Failed to create the material editor window";
		return;
	}

	if (!materialWindow->openMaterialGraph(graphPath))
	{
		MIKAN_LOG_ERROR("MaterialAssetReference::editorOpen") << "Failed to open material graph: " << graphPath;
	}
}

void MaterialAssetReference::editorHandleGraphVariablesDragDrop(const NodeEditorState& editorState)
{
	auto self= std::static_pointer_cast<MaterialAssetReference>(shared_from_this());

	// Create an material property to hold the reference to this asset
	auto materialProperty= editorState.nodeGraph->createTypedProperty<GraphMaterialProperty>();
	materialProperty->setMaterialAssetReference(self);
	materialProperty->setName(editorState.nodeGraph->makeUniquePropertyName(getShortName()));
}

void MaterialAssetReference::editorHandleMainFrameDragDrop(const NodeEditorState& editorState)
{
	auto self= std::static_pointer_cast<MaterialAssetReference>(shared_from_this());

	// Create an material property first to hold the reference to this asset
	auto materialProperty= editorState.nodeGraph->createTypedProperty<GraphMaterialProperty>();
	materialProperty->setMaterialAssetReference(self);
	materialProperty->setName(editorState.nodeGraph->makeUniquePropertyName(getShortName()));

	// Then create a material node in the graph that references the material property
	auto materialNode= editorState.nodeGraph->createTypedNode<MaterialNode>(editorState);
	materialNode->setMaterialSource(materialProperty);
}

void MaterialAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locLabel("assets.materialAssetHeader")))
	{
		const std::string buttonName= StringUtils::stringify(ICON_FK_FOLDER_OPEN, locLabel("assets.material"));

		if (ImGui::SmallButton(buttonName.c_str()))
		{
			static std::string materialPath= MaterialAssetReferenceFactory::getDefaultMaterialPath();
			static const char* filterItems[1]= {"*.mat"};

			const char* picked=
				tinyfd_openFileDialog(locText("assets.loadMaterialDialogTitle"), materialPath.c_str(), 1, filterItems,
									  locText("assets.materialFilterDescription"), 0); // disallow multiple selections

			if (picked != nullptr && picked[0] != '\0')
			{
				setAssetPath(picked);
			}
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
	return (PathUtils::getProjectDirectory() / "shaders" / "compositor" / "").string();
}