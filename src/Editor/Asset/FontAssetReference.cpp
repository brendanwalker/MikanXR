#include "FontAssetReference.h"
#include "PathUtils.h"

// -- FontAssetReferenceFactory -----
FontAssetReferenceFactory::FontAssetReferenceFactory()
	: TypedAssetReferenceFactory<FontAssetReference, AssetReferenceConfig>()
{
	m_defaultPath= (PathUtils::getResourceDirectory() / "font" / "").string();
}

std::filesystem::path FontAssetReferenceFactory::getDefaultFontPath()
{
	return PathUtils::getResourceDirectory() / "font" / "MochiyPopOne-Regular.ttf";
}
