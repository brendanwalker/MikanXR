#include "GraphArrayProperty.h"
#include "Graphs/NodeGraph.h"

GraphArrayProperty::GraphArrayProperty() 
	: GraphProperty() 
{
}

GraphArrayProperty::GraphArrayProperty(NodeGraphPtr ownerGraph) 
	: GraphProperty(ownerGraph) 
{
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
