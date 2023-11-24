#pragma once

#include "NodeEditorFwd.h"
#include <vector>

class NodeGraph
{
public:
	NodeGraph();
	virtual ~NodeGraph();

protected:
	int allocateId();

	std::vector<EditorNodePtr> m_Nodes;
	std::vector<EditorPinPtr> m_Pins;
	std::vector<EditorLinkPtr> m_Links;
	int	m_nextId= 0;

	friend class Node;
	friend class NodeLink;
	friend class NodePin;
};