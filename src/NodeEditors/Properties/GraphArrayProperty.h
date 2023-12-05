#pragma once

#include "GraphProperty.h"

#include <vector>

template <typename t_element_type>
class GraphArrayProperty : public GraphProperty
{
public:
	GraphArrayProperty() : GraphProperty() {}
	GraphArrayProperty(NodeGraphPtr ownerGraph) : GraphProperty(ownerGraph) {}

	const std::vector<t_element_type>& getArray() { return m_array; }
	std::vector<t_element_type>& getArrayMutable() { return m_array; }

protected:
	std::vector<t_element_type> m_array;
};
