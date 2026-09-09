#include "GraphPage.h"
#include "IconsForkAwesome.h"
#include "LocText.h"

// -- GraphPageConfig -----
configuru::Config GraphPageConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["class_name"]= className;
	pt["id"]= id;
	pt["name"]= name;

	return pt;
}

void GraphPageConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className= pt.get_or<std::string>("class_name", GraphPage::k_pageClassName);
	id= pt.get_or<t_graph_page_id>("id", -1);
	name= pt.get_or<std::string>("name", "");
}

// -- GraphPage -----
bool GraphPage::loadFromConfig(GraphPageConfigConstPtr config)
{
	m_id= config->id;
	m_name= config->name;

	return m_id != -1;
}

void GraphPage::saveToConfig(GraphPageConfigPtr config) const
{
	config->className= getClassName();
	config->id= m_id;
	config->name= m_name;
}

const char* GraphPage::editorGetIcon() const { return ICON_FK_FILE_O; }

// -- GraphPageFactory -----
std::string GraphPageFactory::editorGetCreateLabel() const { return locText("nodeEditor.addPage"); }
