#pragma once
#include "Node.h"

class TimeNode : public Node
{
public:
	TimeNode();
	TimeNode(NodeGraphPtr parentGraph);

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Time"; }
};

class TimeNodeFactory : public NodeFactory
{
public:
	TimeNodeFactory()= default;
	TimeNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};