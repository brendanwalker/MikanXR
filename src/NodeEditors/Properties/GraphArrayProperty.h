#pragma once

#include "GraphProperty.h"

#include <vector>

class GraphArrayPropertyConfig : public GraphPropertyConfig
{
public:
	GraphArrayPropertyConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::vector<t_graph_property_id> childPropertyIds;
};

class GraphArrayProperty : public GraphProperty
{
public:
	GraphArrayProperty() = default;

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,
								const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(class GraphPropertyConfig& config) const override;

	inline const std::vector<GraphPropertyPtr>& getArray() { return m_array; }
	inline std::vector<GraphPropertyPtr>& getArrayMutable() { return m_array; }

	bool addProperty(GraphPropertyPtr property);
	bool removeProperty(GraphPropertyPtr property);

protected:
	std::vector<GraphPropertyPtr> m_array;
};

using GraphArrayPropertyFactory= TypedGraphPropertyFactory<GraphArrayProperty, GraphArrayPropertyConfig>;