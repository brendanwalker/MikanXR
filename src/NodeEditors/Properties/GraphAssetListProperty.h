#pragma once

#include "GraphProperty.h"
#include "AssetFwd.h"

#include <vector>

class GraphAssetListPropertyConfig : public GraphPropertyConfig
{
public:
	GraphAssetListPropertyConfig() : GraphPropertyConfig() {}
	GraphAssetListPropertyConfig(const std::string& graphName) : GraphPropertyConfig(graphName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::vector<AssetReferenceConfigPtr> assetRefConfigs;
};

class GraphAssetListProperty : public GraphProperty
{
public:
	GraphAssetListProperty();
	GraphAssetListProperty(NodeGraphPtr ownerGraph);

	virtual bool loadFromConfig(const class GraphPropertyConfig& config) override;
	virtual void saveToConfig(class GraphPropertyConfig& config) const override;

	inline const std::vector<AssetReferencePtr>& getAssetList() { return m_assetReferences; }
	inline std::vector<AssetReferencePtr>& getAssetListMutable() { return m_assetReferences; }

protected:
	std::vector<AssetReferencePtr> m_assetReferences;
};
