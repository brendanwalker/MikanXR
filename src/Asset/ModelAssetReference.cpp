#include "ModelAssetReference.h"
#include "PathUtils.h"

// -- ModelAssetReference -----
void ModelAssetReference::rebuildPreview()
{
	//TODO
}

void ModelAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// TODO: Render material asset properties 
}

// -- ModelAssetReferenceFactory -----
ModelAssetReferenceFactory::ModelAssetReferenceFactory()
	: TypedAssetReferenceFactory<ModelAssetReference, AssetReferenceConfig>()
{
	m_defaultPath = (PathUtils::getResourceDirectory() / "models" / "").string();
}