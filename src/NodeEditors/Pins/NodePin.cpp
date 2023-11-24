#include "NodePin.h"
#include "Nodes/Node.h"
#include "Graphs/NodeGraph.h"

#include "imnodes.h"

NodePin::NodePin() 
	: m_id(-1)
	, m_direction(eNodePinDirection::INPUT)
{
}

NodePin::NodePin(NodePtr ownerNode)
	: m_id(ownerNode->getOwnerGraph()->allocateId())
	, m_size(1)
	, m_direction(eNodePinDirection::INPUT)
{
}

ImNodesPinShape NodePin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(252, 200, 35, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(255, 217, 140, alpha * 255));

	return pinShape;	
}

void NodePin::editorRenderEndPin()
{
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}