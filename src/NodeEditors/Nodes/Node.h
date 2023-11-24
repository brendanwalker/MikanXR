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
	Node(NodeGraphPtr ownerGraph);

	inline int getId() const { return m_id; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	inline const glm::vec2& getNodePos() const { return m_nodePos; }
	inline void setNodePos(const glm::vec2& nodePos) { m_nodePos= nodePos; }

	virtual void editorRender(class NodeEditorState* editorState) {}

protected:
	int m_id;
	NodeGraphPtr m_ownerGraph;
	std::vector<NodePinPtr> m_pinsIn;
	std::vector<NodePinPtr> m_pinsOut;
	glm::vec2 m_nodePos;
};