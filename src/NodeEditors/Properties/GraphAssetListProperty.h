#pragma once

#include "GraphProperty.h"
#include "AssetFwd.h"

#include <vector>

class GraphAssetListProperty : public GraphProperty
{
public:
	GraphAssetListProperty();
	GraphAssetListProperty(NodeGraphPtr ownerGraph);

	inline const std::vector<AssetReferencePtr>& getAssetList() { return m_assets; }
	inline std::vector<AssetReferencePtr>& getAssetListMutable() { return m_assets; }

protected:
	std::vector<AssetReferencePtr> m_assets;
};
