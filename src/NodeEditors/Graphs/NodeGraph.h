#pragma once

#include "NodeFwd.h"
#include <map>

class NodeGraph
{
public:
	NodeGraph();
	virtual ~NodeGraph();

	NodePtr getNodeById(int id);
	NodePinPtr getNodePinById(int id);
	NodeLinkPtr getNodeLinkById(int id);

	virtual void editorRender(class NodeEditorState* editorState);

protected:
	int allocateId();

	std::map<int, NodePtr> m_Nodes;
	std::map<int, NodePinPtr> m_Pins;
	std::map<int, NodeLinkPtr> m_Links;
	int	m_nextId= 0;

	friend class Node;
	friend class NodeLink;
	friend class NodePin;
};