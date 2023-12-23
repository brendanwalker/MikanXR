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
bool GraphArrayProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	NodeGraphPtr graph= getOwnerGraph();

	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		bool loadedAllProps = true;

		auto arrayPropConfig= std::static_pointer_cast<const GraphArrayPropertyConfig>(propConfig);
		for (t_graph_property_id childPropId : arrayPropConfig->childPropertyIds)
		{
			// Fetch the child property if it was already created
			auto childProperty = getOwnerGraph()->getPropertyById(childPropId);

			// If it's not created yet, tell the graph to create it now
			if (!childProperty)
			{
				auto it= graphConfig.propertyConfigMap.find(childPropId);
				if (it != graphConfig.propertyConfigMap.end())
				{
					GraphPropertyConfigPtr childPropertyConfig= it->second;

					childProperty= graph->loadGraphPropertyFromConfig(childPropertyConfig, graphConfig);
				}
			}

			// Add it to the array
			if (childProperty)
			{
				m_array.push_back(childProperty);
			}
			else
			{
				MIKAN_LOG_ERROR("GraphArrayProperty::loadFromConfig") 
								<< "Invalid property id: " << childPropId 
								<< ", on array: " << getName();
				loadedAllProps= false;
			}
		}

		return loadedAllProps;
	}

	return false;
}

void GraphArrayProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto& propConfig = std::static_pointer_cast<GraphArrayPropertyConfig>(config);

	for (auto property : m_array)
	{
		propConfig->childPropertyIds.push_back(property->getId());
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
