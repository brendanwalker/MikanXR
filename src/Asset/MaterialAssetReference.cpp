#include "MaterialAssetReference.h"
#include "PathUtils.h"

// -- MaterialAssetReference -----
void MaterialAssetReference::rebuildPreview()
{
	//TODO
}

void MaterialAssetReference::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// TODO: Render material asset properties 
}

// -- MaterialAssetReferenceFactory -----
MaterialAssetReferenceFactory::MaterialAssetReferenceFactory()
	: TypedAssetReferenceFactory<MaterialAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getResourceDirectory() / "shaders" / "compositor" / "").string();
}