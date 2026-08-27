#include "GraphProperty.h"
#include "Graphs/NodeGraph.h"
#include "MkGuiDrawUtils.h"

#include "imgui.h"

// -- GraphPropertyConfig ------
configuru::Config GraphPropertyConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["class_name"]= className;
	pt["id"]= id;
	pt["parent_id"]= parentId;
	pt["name"]= name;
	pt["sort_order"]= sortOrder;

	return pt;
}

void GraphPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className= pt.get_or<std::string>("class_name", "GraphProperty");
	id= pt.get_or<t_graph_property_id>("id", -1);
	parentId= pt.get_or<t_graph_property_id>("parent_id", -1);
	name= pt.get_or<std::string>("name", "");
	sortOrder= pt.get_or<int>("sort_order", -1);
}

// -- GraphProperty ------
std::string GraphProperty::editorGetIcon() const { return MkGui::getVariableIcon(); }

const ImVec4 GraphProperty::editorGetIconColor() const { return MkGui::getPropertyColor(); }

bool GraphProperty::loadFromConfig(GraphPropertyConfigConstPtr propConfig, const NodeGraphConfig& graphConfig)
{
	m_id= propConfig->id;
	m_parentId= propConfig->parentId;
	m_name= propConfig->name;
	m_sortOrder= propConfig->sortOrder;

	return true;
}

void GraphProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	config->className= getClassName();
	config->id= m_id;
	config->parentId= m_parentId;
	config->name= m_name;
	config->sortOrder= m_sortOrder;
}

void GraphProperty::notifyPropertyModified() const
{
	if (m_ownerGraph && m_ownerGraph->OnPropertyModifed)
	{
		m_ownerGraph->OnPropertyModifed(m_id);
	}
}

// -- GraphPropertyFactory ------
GraphPropertyConfigPtr GraphPropertyFactory::allocatePropertyConfig() const
{
	return std::make_shared<GraphPropertyConfig>();
}

GraphPropertyPtr GraphPropertyFactory::allocateProperty() const { return std::make_shared<GraphProperty>(); }