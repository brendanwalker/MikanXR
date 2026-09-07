#include "PixelContentAssetReference.h"
#include "PathUtils.h"

// -- PixelContentAssetReferenceFactory -----
PixelContentAssetReferenceFactory::PixelContentAssetReferenceFactory()
	: TypedAssetReferenceFactory<PixelContentAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getProjectDirectory() / "textures" / "").string();
}
