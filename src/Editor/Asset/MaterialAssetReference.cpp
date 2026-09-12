#include "MaterialAssetReference.h"
#include "App.h"
#include "Logger.h"
#include "MikanShaderConfig.h"
#include "NodeEditorState.h"
#include "PathUtils.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"

#include "Graphs/NodeGraph.h"
#include "Nodes/MaterialNode.h"
#include "Properties/GraphMaterialProperty.h"
#include "Windows/MaterialNodeEditorWindow.h"

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

	// One material editor window serves every material, so reuse the open one,
	// raised so the open is visible from behind the editor that asked for it
	App* app= App::getInstance();
	MaterialNodeEditorWindow* materialWindow= app->getWindowOfType<MaterialNodeEditorWindow>();
	if (materialWindow == nullptr)
	{
		materialWindow= app->createAppWindow<MaterialNodeEditorWindow>();
	}
	else
	{
		materialWindow->getMkWindowContext()->raiseWindow();
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
	MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
								   locLabel("assets.materialAssetHeader"));
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