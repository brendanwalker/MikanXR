#pragma once

#include "Node.h"

class ArrayNodeConfig : public NodeConfig
{
public:
	ArrayNodeConfig() = default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string elementClassName;
};

class ArrayNode : public Node
{
public:
	ArrayNode() = default;

	inline static const std::string k_nodeClassName = "ArrayNode";
	virtual std::string getClassName() const { return k_nodeClassName; }

	inline const std::string& getElementClassName() const { return m_elementClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig);
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const;

	virtual bool evaluateNode(NodeEvaluator& evaluator);

	virtual std::string editorGetTitle() const { return "Make Array"; }

protected:
	virtual void editorRenderInputPins(const NodeEditorState& editorState) override;
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;
	void setElementClassName(const std::string inElementClassName);
	void rebuildOutputArrayValue();

protected:
	std::string m_elementClassName;
};

class ArrayNodeFactory : public TypedNodeFactory<ArrayNode, ArrayNodeConfig>
{
public:
	ArrayNodeFactory() = default;

	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};