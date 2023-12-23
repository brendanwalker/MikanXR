#include "AssetReference.h"

// -- Asset Reference Config -----
configuru::Config AssetReferenceConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["class_name"] = className;
	pt["asset_path"] = assetPath;

	return pt;
}

void AssetReferenceConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className = pt.get_or<std::string>("class_name", "AssetReference");
	assetPath = pt.get_or<std::string>("asset_path", "");
}

// -- Asset Reference -----
AssetReference::~AssetReference()
{
	m_previewTexture= nullptr;
}

bool AssetReference::loadFromConfig(const class AssetReferenceConfig& config)
{
	setAssetPath(config.assetPath);

	return true;
}

void AssetReference::saveToConfig(class AssetReferenceConfig& config) const
{
	config.className = typeid(*this).name();
	config.assetPath = m_assetPath.string();
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
AssetReferenceConfigPtr AssetReferenceFactory::allocateAssetReferenceConfig() const
{
	return std::make_shared<AssetReferenceConfig>();
}

AssetReferencePtr AssetReferenceFactory::allocateAssetReference() const
{
	return std::make_shared<AssetReference>();
}