#pragma once
#include "LocText.h"
#include "Node.h"

class TimeNode : public Node
{
public:
	TimeNode()= default;

	inline static const std::string k_nodeClassName= "TimeNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual std::string editorGetTitle() const override { return locText("nodes.timeTitle"); }

	float m_currentTime= 0.f;
};

class TimeNodeFactory : public TypedNodeFactory<TimeNode, NodeConfig>
{
public:
	TimeNodeFactory()= default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};