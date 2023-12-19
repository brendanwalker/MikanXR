#include "GraphProperty.h"
#include "Graphs/NodeGraph.h"

// -- GraphPropertyConfig ------
configuru::Config GraphPropertyConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["class_name"] = className;
	pt["id"] = id;
	pt["parent_id"] = parentId;
	pt["name"] = name;

	return pt;
}

void GraphPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className = pt.get_or<std::string>("class_name", "Node");
	id = pt.get_or<t_graph_property_id>("id", -1);
	parentId = pt.get_or<t_graph_property_id>("parent_id", -1);
	name = pt.get_or<std::string>("name", "");
}

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

bool GraphProperty::loadFromConfig(const class GraphPropertyConfig& config)
{
	m_id= config.id;
	m_parentId= config.parentId;
	m_name= config.name;

	return true;
}

void GraphProperty::saveToConfig(class GraphPropertyConfig& config) const
{
	config.className= typeid(*this).name();
	config.id= m_id;
	config.parentId= m_parentId;
	config.name= m_name;
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