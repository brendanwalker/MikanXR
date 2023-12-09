#include "CompositorNodeGraph.h"
#include "Nodes/DrawTriMeshNode.h"
#include "Nodes/EventNode.h"
#include "Nodes/MousePosNode.h"
#include "Nodes/TextureNode.h"
#include "Nodes/TimeNode.h"

CompositorNodeGraph::CompositorNodeGraph() : NodeGraph()
{
	NodeGraphPtr ownerGraph= shared_from_this();

	// Nodes this graph can spawn
	m_nodeFactories.push_back(NodeFactory::create<DrawTriMeshNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<EventNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<MousePosNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<TextureNodeFactory>(ownerGraph));
	m_nodeFactories.push_back(NodeFactory::create<TimeNodeFactory>(ownerGraph));
}