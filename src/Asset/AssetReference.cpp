#include "AssetReference.h"

// -- Asset Reference -----
AssetReference::~AssetReference()
{
	m_previewTexture= nullptr;
}

void AssetReference::setAssetPath(const std::filesystem::path& inPath)
{
	if (m_assetPath != inPath)
	{
		m_assetPath= inPath;
		rebuildPreview();
	}
}

bool AssetReference::isValid() const
{
	return !m_assetPath.empty() && std::filesystem::exists(m_assetPath);
}

std::string AssetReference::getShortName() const
{
	if (!m_assetPath.empty() && m_assetPath.has_filename())
	{
		return m_assetPath.filename().string();
	}

	return "";
}

// -- AssetReferenceFactory ----
std::shared_ptr<AssetReference> AssetReferenceFactory::createAssetReference(
	const NodeEditorState* editorState,
	const std::filesystem::path& inAssetPath) const
{
	auto assetRef= std::make_shared<AssetReference>();
	assetRef->setAssetPath(inAssetPath);

	return assetRef;
}