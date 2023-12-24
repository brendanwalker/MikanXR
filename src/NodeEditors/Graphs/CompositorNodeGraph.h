#pragma once

#include "NodeGraph.h"
#include <string>

class CompositorNodeGraph : public NodeGraph
{
public:
	CompositorNodeGraph();

	static const std::string k_compositeFrameEventName;

	virtual bool loadFromConfig(const NodeGraphConfig& config) override;
	bool compositeFrame(NodeEvaluator& evaluator);

protected:
	NodePtr m_compositeFrameEventNode;
};

class CompositorNodeGraphFactory : public TypedNodeGraphFactory<CompositorNodeGraph>
{
public:
	CompositorNodeGraphFactory() = default;

	virtual NodeGraphPtr initialCreateNodeGraph() const override;
};
