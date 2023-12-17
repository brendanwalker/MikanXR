#pragma once

#include "NodeGraph.h"

class CompositorNodeGraphFactory : public NodeGraphFactory
{
public:
	CompositorNodeGraphFactory() : NodeGraphFactory() {}

protected:
	virtual NodeGraphPtr allocateNodeGraph() const override;
};

class CompositorNodeGraph : public NodeGraph
{
public:
	CompositorNodeGraph();
};
