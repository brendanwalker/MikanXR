#pragma once

#include "NodeFwd.h"
#include "glm/ext/vector_float2.hpp"

#include <string>
#include <vector>
#include <stdint.h>

class Node
{
public:
	Node();
	Node(NodeGraphPtr parentGraph);

	inline NodeGraphPtr getParentGraph() const { return m_parentGraph; }
	virtual void onEditorRenderNode(class NodeEditorState* editorState) {}

protected:
	int m_id;
	NodeGraphPtr m_parentGraph;
	std::vector<NodePinPtr> m_pinsIn;
	std::vector<NodePinPtr> m_pinsOut;
	glm::vec2 m_nodePos;
};