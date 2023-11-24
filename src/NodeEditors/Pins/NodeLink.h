#pragma once

#include "NodeFwd.h"

class NodeLink
{
public:
	NodeLink();
	NodeLink(NodeGraphPtr m_ownerGraph);

protected:
	int m_id;
	NodeGraphPtr m_ownerGraph;
	NodePinPtr m_pPin1;
	NodePinPtr m_pPin2;
};