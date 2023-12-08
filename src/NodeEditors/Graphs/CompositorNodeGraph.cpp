#include "CompositorNodeGraph.h"
#include "Nodes/TimeNode.h"

CompositorNodeGraph::CompositorNodeGraph() : NodeGraph()
{
	NodeGraphPtr ownerGraph= shared_from_this();

	// Nodes this graph can spawn
	m_nodeFactories.push_back(NodeFactory::create<TimeNodeFactory>(ownerGraph));
}