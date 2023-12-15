#pragma once

#include "NodeFwd.h"

#include <string>

class GraphProperty : public std::enable_shared_from_this<GraphProperty>
{
public:
	GraphProperty();
	GraphProperty(NodeGraphPtr ownerGraph);
	virtual ~GraphProperty() {}

	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	inline t_graph_property_id getId() const { return m_id; }

	inline void setName(const std::string& name) { m_name= name; }
	inline const std::string& getName() const { return m_name; }

	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) {}
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) {}
	void notifyPropertyModified() const;

protected:
	t_graph_property_id m_id;
	std::string m_name;
	NodeGraphPtr m_ownerGraph;
};

class GraphPropertyFactory
{
public:
	GraphPropertyFactory() = default;
	GraphPropertyFactory(NodeGraphPtr ownerGraph);

	inline NodeGraphPtr getOwner() const { return m_ownerGraph; }

	virtual const std::string getPropertyTypeName() const { return "property"; }
	virtual GraphPropertyPtr createProperty(
		const class NodeEditorState* editorState,
		const std::string& name) const;

	template <class t_property_factory_class>
	static GraphPropertyFactoryPtr create(NodeGraphPtr ownerGraph)
	{
		// Create a node factory instance
		return std::make_shared<t_property_factory_class>(ownerGraph);
	}

protected:
	NodeGraphPtr m_ownerGraph;
};