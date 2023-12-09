#pragma once
#include "Node.h"

class MousePosNode : public Node
{
public:
	MousePosNode();
	MousePosNode(NodeGraphPtr parentGraph);

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "MousePosition"; }
};

class MousePosNodeFactory : public NodeFactory
{
public:
	MousePosNodeFactory() = default;
	MousePosNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};