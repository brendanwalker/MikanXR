#include "ShapeGraphAssetReference.h"
#include "PathUtils.h"

// -- ShapeGraphAssetReferenceFactory -----
ShapeGraphAssetReferenceFactory::ShapeGraphAssetReferenceFactory()
	: TypedAssetReferenceFactory<ShapeGraphAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getProjectDirectory() / "shapes" / "").string();
}
