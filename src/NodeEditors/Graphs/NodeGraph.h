#pragma once

#include "NodeFwd.h"
#include "MulticastDelegate.h"

#include <map>
#include <string>

class NodeGraph : public std::enable_shared_from_this<NodeGraph>
{
public:
	NodeGraph()= default;
	virtual ~NodeGraph() {}

	inline float getTimeInSeconds() const { return m_timeInSeconds; }

	GraphPropertyPtr getPropertyById(t_graph_property_id id) const;
	GraphPropertyPtr getPropertyByName(const std::string& name) const;

	template <class t_property_type>
	std::shared_ptr<t_property_type> getTypedPropertyById(t_graph_property_id id) const
	{
		return std::dynamic_pointer_cast<t_property_type>(getPropertyById(id));
	}

	template <class t_property_type>
	std::shared_ptr<t_property_type> getTypedPropertyByName(const std::string& name) const
	{
		return std::dynamic_pointer_cast<t_property_type>(getPropertyByName(name));
	}

	MulticastDelegate<void(t_graph_property_id id)> OnPropertyAdded;
	MulticastDelegate<void(t_graph_property_id id)> OnPropertyModifed;
	MulticastDelegate<void(t_graph_property_id id)> OnPropertyDeleted;

	template <class _Pr>
	NodePtr getNodeByPredicate(_Pr predicate) const
	{
		auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), predicate);
		if (it != m_Nodes.end())
		{
			return it->second;
		}

		return NodePtr();
	}
	NodePtr getNodeById(t_node_id id) const;
	NodePtr getEventNodeByName(const std::string& eventName) const;

	NodePinPtr getNodePinById(t_node_pin_id id) const;
	NodeLinkPtr getNodeLinkById(t_node_link_id id) const;

	virtual void update(class NodeEvaluator& evaluator);

	NodePtr createNode(NodeFactoryPtr nodeFactory, const NodeEditorState* nodeEditorState);
	MulticastDelegate<void(t_node_id id)> OnNodeCreated;

	bool deleteNodeById(t_node_id id);
	MulticastDelegate<void(t_node_id id)> OnNodeDeleted;

	bool deletePinById(t_node_pin_id id);
	MulticastDelegate<void(t_node_pin_id id)> OnPinDeleted;

	NodeLinkPtr createLink(t_node_pin_id startPinId, t_node_pin_id endPinId);
	MulticastDelegate<void(t_node_link_id id)> OnLinkCreated;

	bool deleteLinkById(t_node_link_id id);
	MulticastDelegate<void(t_node_link_id id)> OnLinkDeleted;

	virtual std::vector<NodeFactoryPtr> editorGetValidNodeFactories(const class NodeEditorState& editorState) const;
	virtual void editorRender(const class NodeEditorState& editorState);

protected:
	int allocateId();

	float m_timeInSeconds;

	std::vector<NodeFactoryPtr> m_nodeFactories;
	std::map<t_node_id, NodePtr> m_Nodes;
	std::map<t_node_pin_id, NodePinPtr> m_Pins;
	std::map<t_node_link_id, NodeLinkPtr> m_Links;
	std::map<t_graph_property_id, GraphPropertyPtr> m_properties;
	int	m_nextId= 0;

	friend class Node;
	friend class NodeLink;
	friend class NodePin;
	friend class GraphProperty;
};