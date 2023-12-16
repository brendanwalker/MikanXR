#include "GraphProperty.h"
#include "Graphs/NodeGraph.h"

// -- GraphProperty ------
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

// -- GraphPropertyFactory ------
GraphPropertyFactory::GraphPropertyFactory(NodeGraphPtr ownerGraph)
	: m_ownerGraph(ownerGraph)
{

}

GraphPropertyPtr GraphPropertyFactory::createProperty(
	const NodeEditorState* editorState,
	const std::string& name) const
{
	return m_ownerGraph->addTypedProperty<GraphProperty>(name);
}