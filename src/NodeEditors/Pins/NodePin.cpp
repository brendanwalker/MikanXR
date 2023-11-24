#include "NodePin.h"
#include "Nodes/Node.h"
#include "Graphs/NodeGraph.h"

NodePin::NodePin() 
	: m_id(-1)
	, m_direction(eNodePinDirection::INPUT)
{
}

NodePin::NodePin(NodePtr ownerNode)
	: m_id(ownerNode->getParentGraph()->allocateId())
	, m_size(1)
	, m_direction(eNodePinDirection::INPUT)
{
}