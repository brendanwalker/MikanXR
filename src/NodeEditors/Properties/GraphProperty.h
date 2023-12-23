#pragma once

#include "NodeFwd.h"
#include "CommonConfig.h"

#include <string>

class GraphPropertyConfig : public CommonConfig
{
public:
	GraphPropertyConfig() : CommonConfig() {}
	GraphPropertyConfig(const std::string& nodeName) : CommonConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string className;
	t_graph_property_id id= -1;
	t_graph_property_id parentId= -1;
	std::string name;
};

class GraphProperty : public std::enable_shared_from_this<GraphProperty>
{
public:
	GraphProperty()= default;
	virtual ~GraphProperty() {}

	virtual bool loadFromConfig(
		GraphPropertyConfigConstPtr propConfig,
		const NodeGraphConfig& graphConfig);
	virtual void saveToConfig(class GraphPropertyConfig& config) const;

	inline void setOwnerGraph(NodeGraphPtr ownerGraph) { m_ownerGraph= ownerGraph; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	
	inline void setId(t_graph_property_id id) { m_id= id; }
	inline t_graph_property_id getId() const { return m_id; }

	inline void setParentId(t_graph_property_id inParentId ) { m_parentId= inParentId; }
	inline t_graph_property_id getParentId() const { return m_parentId; }

	inline void setName(const std::string& name) { m_name= name; }
	inline const std::string& getName() const { return m_name; }

	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) {}
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) {}
	void notifyPropertyModified() const;

protected:
	t_graph_property_id m_id;
	t_graph_property_id m_parentId;
	std::string m_name;
	NodeGraphPtr m_ownerGraph;
};

class GraphPropertyFactory
{
public:
	GraphPropertyFactory() = default;

	inline std::string getGraphPropertyClassName() const { 
		return typeid(m_defaultGraphPropertyObject.get()).name(); 
	}

	virtual GraphPropertyConfigPtr allocatePropertyConfig() const;
	virtual GraphPropertyPtr allocateProperty() const;

	template <class t_property_factory_class>
	static GraphPropertyFactoryPtr createFactory()
	{
		// Create a node factory instance
		auto factory= std::make_shared<t_property_factory_class>();

		factory->m_defaultGraphPropertyObject= factory->allocateProperty();

		return factory;
	}

protected:
	GraphPropertyPtr m_defaultGraphPropertyObject;
};

template <class t_property_class, class t_property_config_class>
class TypedGraphPropertyFactory : public GraphPropertyFactory
{
public:
	TypedGraphPropertyFactory() = default;

	virtual GraphPropertyConfigPtr allocatePropertyConfig() const override
	{
		return std::make_shared<t_property_config_class>();
	}

	virtual GraphPropertyPtr allocateProperty() const override
	{
		return std::make_shared<t_property_class>();
	}
};