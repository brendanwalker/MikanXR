#pragma once

#include "GraphProperty.h"

#include <vector>

class GraphArrayProperty : public GraphProperty
{
public:
	GraphArrayProperty();
	GraphArrayProperty(NodeGraphPtr ownerGraph);

	inline const std::vector<GraphPropertyPtr>& getArray() { return m_array; }
	inline std::vector<GraphPropertyPtr>& getArrayMutable() { return m_array; }

protected:
	std::vector<GraphPropertyPtr> m_array;
};

