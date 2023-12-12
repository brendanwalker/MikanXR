#pragma once

#include "NodeFwd.h"

#include <string>

class GraphProperty
{
public:
	GraphProperty();
	GraphProperty(NodeGraphPtr ownerGraph);
	virtual ~GraphProperty() {}

	inline t_graph_property_id getId() const { return m_id; }

	inline void setName(const std::string& name) { m_name= name; }
	inline const std::string& getName() const { return m_name; }

	void notifyPropertyModified() const;

protected:
	t_graph_property_id m_id;
	std::string m_name;
	NodeGraphPtr m_ownerGraph;
};