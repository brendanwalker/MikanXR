#include "MkNodesScopedNode.h"

#include "imnodes.h"

MkNodesScopedNode::MkNodesScopedNode(int id)
{
	ImNodes::BeginNode(id);
}

MkNodesScopedNode::~MkNodesScopedNode()
{
	ImNodes::EndNode();
}
