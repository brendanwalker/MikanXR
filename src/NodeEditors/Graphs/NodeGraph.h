#pragma once

#include "NodeFwd.h"
#include "MulticastDelegate.h"

#include <map>

class NodeGraph
{
public:
	NodeGraph();
	virtual ~NodeGraph();

	NodePtr getNodeById(t_node_id id) const;
	NodePinPtr getNodePinById(t_node_pin_id id) const;
	NodeLinkPtr getNodeLinkById(t_node_link_id id) const;

	bool deleteNodeById(t_node_id id);
	MulticastDelegate<void(t_node_id id)> OnNodeDeleted;

	bool deletePinById(t_node_pin_id id);
	MulticastDelegate<void(t_node_pin_id id)> OnPinDeleted;

	bool deleteLinkById(t_node_link_id id);
	MulticastDelegate<void(t_node_link_id id)> OnLinkDeleted;

	virtual void editorRender(class NodeEditorState* editorState);

protected:
	int allocateId();

	std::map<t_node_id, NodePtr> m_Nodes;
	std::map<t_node_pin_id, NodePinPtr> m_Pins;
	std::map<t_node_link_id, NodeLinkPtr> m_Links;
	int	m_nextId= 0;

	friend class Node;
	friend class NodeLink;
	friend class NodePin;
};