#pragma once
#include "Node.h"

class MousePosNode : public Node
{
public:
	MousePosNode()= default;

	virtual const std::string& getClassName() const override { return "MousePosNode"; }
	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "MousePosition"; }
};

class MousePosNodeFactory : public TypedNodeFactory<MousePosNode, NodeConfig>
{
public:
	MousePosNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};