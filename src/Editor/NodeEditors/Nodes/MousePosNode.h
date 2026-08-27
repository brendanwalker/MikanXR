#pragma once
#include "LocText.h"
#include "Node.h"

class MousePosNode : public Node
{
public:
	MousePosNode()= default;

	inline static const std::string k_nodeClassName= "MousePosNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;
	virtual std::string editorGetTitle() const override { return locText("nodes.mousePositionTitle"); }
};

class MousePosNodeFactory : public TypedNodeFactory<MousePosNode, NodeConfig>
{
public:
	MousePosNodeFactory()= default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};