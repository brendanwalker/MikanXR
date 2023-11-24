#include "Node.h"
#include "Graphs/NodeGraph.h"

Node::Node()
	: m_id(-1)
	, m_nodePos(glm::vec2(0.f))
{

}

Node::Node(NodeGraphPtr parentGraph)
	: m_id(parentGraph->allocateId())
	, m_nodePos(glm::vec2(0.f))
{
	
}