#include "NodeGraphAssetReference.h"
#include "PathUtils.h"

// -- MaterialAssetReference -----
void NodeGraphAssetReference::rebuildPreview()
{
	//TODO
}

void NodeGraphAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// TODO: Render material asset properties 
}

// -- NodeGraphAssetReferenceFactory -----
NodeGraphAssetReferenceFactory::NodeGraphAssetReferenceFactory()
	: TypedAssetReferenceFactory<NodeGraphAssetReference, AssetReferenceConfig>()
{
	m_defaultPath = (PathUtils::getResourceDirectory() / "graphs" / "").string();
}