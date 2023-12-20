#include "GraphAssetListProperty.h"
#include "AssetReference.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"

// -- GraphAssetListPropertyConfig -----
configuru::Config GraphAssetListPropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	writeStdConfigVector(pt, "assetReferences", assetRefConfigs);

	return pt;
}

void GraphAssetListPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	GraphPropertyConfig::readFromJSON(pt);

	readStdConfigVector(pt, "assetReferences", assetRefConfigs);
}

// -- GraphAssetListProperty -----
GraphAssetListProperty::GraphAssetListProperty()
	: GraphProperty()
{}

GraphAssetListProperty::GraphAssetListProperty(NodeGraphPtr ownerGraph)
	: GraphProperty(ownerGraph)
{}

bool GraphAssetListProperty::loadFromConfig(const GraphPropertyConfig& config)
{
	NodeGraphPtr ownerGraph= getOwnerGraph();
	const auto& assetListconfig= static_cast<const GraphAssetListPropertyConfig&>(config);
	bool success= true;

	// Load all asset references
	for (auto assetRefConfig : assetListconfig.assetRefConfigs)
	{
		AssetReferenceFactoryPtr factory = ownerGraph->getAssetReferenceFactory(assetRefConfig->className);
		if (factory)
		{
			AssetReferencePtr assetReference = factory->createAssetReference(nullptr, "");
			if (assetReference)
			{
				if (assetReference->loadFromConfig(*assetRefConfig.get()))
				{
					m_assetReferences.push_back(assetReference);
				}
				else
				{
					MIKAN_LOG_INFO("GraphAssetListProperty::loadFromConfig") << "Failed to load asset reference: " << assetRefConfig->className;
					success= false;
				}
			}
			else
			{
				MIKAN_LOG_INFO("GraphAssetListProperty::loadFromConfig") << "Failed to create asset reference: " << assetRefConfig->className;
				success= false;
			}
		}
		else
		{
			MIKAN_LOG_INFO("GraphAssetListProperty::loadFromConfig") << "Unknown asset reference class: " << assetRefConfig->className;
			success= false;
		}
	}

	return success;
}

void GraphAssetListProperty::saveToConfig(class GraphPropertyConfig& config) const
{
	// TODO
}