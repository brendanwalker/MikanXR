#pragma once

#include "GraphProperty.h"

#include <vector>

class GraphArrayPropertyConfig : public GraphPropertyConfig
{
public:
	GraphArrayPropertyConfig() : GraphPropertyConfig() {}
	GraphArrayPropertyConfig(const std::string& nodeName) : GraphPropertyConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::vector<t_graph_property_id> childPropertyIds;
};

class GraphArrayProperty : public GraphProperty
{
public:
	GraphArrayProperty();
	GraphArrayProperty(NodeGraphPtr ownerGraph);

	virtual bool loadFromConfig(const class GraphPropertyConfig& config) override;
	virtual void saveToConfig(class GraphPropertyConfig& config) const override;

	inline const std::vector<GraphPropertyPtr>& getArray() { return m_array; }
	inline std::vector<GraphPropertyPtr>& getArrayMutable() { return m_array; }

	bool addProperty(GraphPropertyPtr property);
	bool removeProperty(GraphPropertyPtr property);

protected:
	std::vector<GraphPropertyPtr> m_array;
};

