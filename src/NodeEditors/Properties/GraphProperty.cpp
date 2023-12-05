#include "GraphProperty.h"
#include "Graphs/NodeGraph.h"

GraphProperty::GraphProperty() 
	: m_id(-1)
{
}

GraphProperty::GraphProperty(NodeGraphPtr ownerGraph)
	: m_id(ownerGraph->allocateId())
	, m_ownerGraph(ownerGraph)
{
}

void GraphProperty::notifyPropertyModified() const
{
	if (m_ownerGraph && m_ownerGraph->OnPropertyModifed)
	{
		m_ownerGraph->OnPropertyModifed(m_id);
	}
}