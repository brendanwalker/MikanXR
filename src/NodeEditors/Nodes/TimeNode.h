#pragma once
#include "Node.h"

class TimeNode : public Node
{
public:
	TimeNode() = default;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Time"; }

	float m_currentTime= 0.f;
};

class TimeNodeFactory : public TypedNodeFactory<TimeNode, NodeConfig>
{
public:
	TimeNodeFactory()= default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};