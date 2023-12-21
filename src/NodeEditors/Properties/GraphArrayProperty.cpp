#include "GraphArrayProperty.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"

// -- GraphArrayPropertyConfig -----
configuru::Config GraphArrayPropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	writeStdValueVector(pt, "child_property_ids", childPropertyIds);

	return pt;
}

void GraphArrayPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	readStdValueVector(pt, "child_property_ids", childPropertyIds);

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphArrayProperty -----
GraphArrayProperty::GraphArrayProperty() 
	: GraphProperty() 
{
}

GraphArrayProperty::GraphArrayProperty(NodeGraphPtr ownerGraph) 
	: GraphProperty(ownerGraph) 
{
}

bool GraphArrayProperty::loadFromConfig(const GraphPropertyConfig& config)
{
	if (GraphProperty::loadFromConfig(config))
	{
		bool success= true;

		const auto& propConfig = static_cast<const GraphArrayPropertyConfig&>(config);
		for (t_graph_property_id propId : propConfig.childPropertyIds)
		{
			auto property = getOwnerGraph()->getPropertyById(propId);

			if (property)
			{
				m_array.push_back(property);
			}
			else
			{
				MIKAN_LOG_ERROR("GraphArrayProperty::loadFromConfig") 
								<< "Invalid property id: " << propId 
								<< ", on array: " << getName();
				success= false;
			}
		}

		return success;
	}

	return false;
}

void GraphArrayProperty::saveToConfig(GraphPropertyConfig& config) const
{
	auto& propConfig = static_cast<GraphArrayPropertyConfig&>(config);

	for (auto property : m_array)
	{
		propConfig.childPropertyIds.push_back(property->getId());
	}

	GraphProperty::saveToConfig(config);
}

bool GraphArrayProperty::addProperty(GraphPropertyPtr property)
{
	if (property->getParentId() == this->getId())
	{
		return false;
	}

	if (property->getParentId() == -1)
	{
		getOwnerGraph()->getTypedPropertyById<GraphArrayProperty>(property->getParentId());
	}

	// Make the property's parent this array
	property->setParentId(this->getId());

	// Add property to the array
	m_array.push_back(property);

	// Let any change listeners know that this array was modifed
	notifyPropertyModified();

	return true;
}

bool GraphArrayProperty::removeProperty(GraphPropertyPtr property)
{
	if (property->getParentId() != this->getId())
	{
		return false;
	}

	// Make the property forget about this parent
	property->setParentId(-1);

	// Remove from this array
	auto it= std::find(m_array.begin(), m_array.end(), property);
	if (it != m_array.end())
	{
		m_array.erase(it);

		// Let any change listeners know that this array was modifed
		notifyPropertyModified();
	}

	return true;
}
