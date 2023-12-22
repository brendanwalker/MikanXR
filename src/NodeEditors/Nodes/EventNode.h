#pragma once
#include "Node.h"

class EventNodeConfig : public NodeConfig
{
public:
	EventNodeConfig() : NodeConfig() {}
	EventNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string eventName;
};

class EventNode : public Node
{
public:
	EventNode();
	EventNode(NodeGraphPtr parentGraph);

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig);
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const;

	inline void setName(const std::string& inName) { m_eventName= inName; }
	inline const std::string& getName() const { return m_eventName; }

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual bool hasAnyFlowPins() const override { return true; }
	virtual FlowPinPtr getOutputFlowPin() const override;

	virtual bool editorCanDelete() const override { return false; }

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	std::string m_eventName;
};

class EventNodeFactory : public NodeFactory
{
public:
	EventNodeFactory() = default;
	EventNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodeConfigPtr createNodeConfig() const
	{
		return std::make_shared<EventNodeConfig>();
	}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};