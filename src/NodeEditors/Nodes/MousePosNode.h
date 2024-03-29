#pragma once
#include "Node.h"

class MousePosNode : public Node
{
public:
	MousePosNode()= default;

	inline static const std::string k_nodeClassName = "MousePosNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
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