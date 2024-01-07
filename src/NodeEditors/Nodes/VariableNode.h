#pragma once

#include "Node.h"
#include "RendererFwd.h"

enum class eVariableEvalMode
{
	INVALID = -1,

	get,
	set,

	COUNT
};
extern const std::string* k_variableEvalModeStrings;

class VariableNodeConfig : public NodeConfig
{
public:
	VariableNodeConfig() : NodeConfig() {}
	VariableNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_property_id valuePropertyId;
	eVariableEvalMode evalMode;
};

class VariableNode : public Node
{
public:
	VariableNode() = default;
	virtual ~VariableNode();

	inline static const std::string k_nodeClassName = "VariableNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	inline GraphValuePropertyPtr getValueSource() const { return m_sourceProperty; }
	void setValueSource(GraphValuePropertyPtr inValueProperty);

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState);

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override;

	void onGraphPropertyDeleted(t_graph_property_id id);
	void rebuildPins();

protected:
	GraphValuePropertyPtr m_sourceProperty;
	eVariableEvalMode m_evalMode;

	ValuePinPtr m_inputValuePin;
	ValuePinPtr m_outputValuePin;
};

class VariableNodeFactory : public TypedNodeFactory<VariableNode, VariableNodeConfig>
{
public:
	VariableNodeFactory() = default;

	virtual bool editorCanCreate() const override { return false; }
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};