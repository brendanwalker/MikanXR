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
	// Stored form: the bundled font folder is the read-only overlay behind the
	// project, so resolveProjectResource finds it without a project prefix
	return std::filesystem::path("font/MochiyPopOne-Regular.ttf");
}
