#include "NodeLink.h"
#include "Graphs/NodeGraph.h"

NodeLink::NodeLink()
	: m_id(-1)
{

}

NodeLink::NodeLink(NodeGraphPtr ownerGraph)
	: m_id(ownerGraph->allocateId())
	, m_ownerGraph(ownerGraph)
{

}