#pragma once

#include "NodeFwd.h"
#include "CommonConfig.h"
#include "Pins/NodePinConstants.h"
#include "glm/ext/vector_float2.hpp"

#include <string>
#include <vector>
#include <stdint.h>

struct NodeDimensions
{
	float titleWidth= 0.f;
	float inputColomnWidth= 0.f;
	float outputColomnWidth= 0.f;
	float totalNodeWidth= 0.f;
};

class NodeConfig : public CommonConfig
{
public:
	NodeConfig() : CommonConfig() {}
	NodeConfig(const std::string& nodeName) : CommonConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string className;
	t_node_id id;
	std::vector<t_node_pin_id> pinIDsIn;
	std::vector<t_node_pin_id> pinIDsOut;
	std::array<float, 2> pos;
};

class Node : public std::enable_shared_from_this<Node>
{
public:
	Node();
	Node(NodeGraphPtr ownerGraph);
	virtual ~Node() {}

	virtual bool loadFromConfig(const class NodeConfig& config);
	virtual void saveToConfig(class NodeConfig& config) const;

	inline int getId() const { return m_id; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	inline const glm::vec2& getNodePos() const { return m_nodePos; }
	inline const std::vector<NodePinPtr>& getInputPins() const { return m_pinsIn; }
	inline const std::vector<NodePinPtr>& getOutputPins() const { return m_pinsOut; }	

	template <class t_pin_type>
	std::shared_ptr<t_pin_type> getFirstPinOfType(eNodePinDirection direction) const
	{
		const std::vector<NodePinPtr>& pinArray= 
			(direction == eNodePinDirection::INPUT) ? m_pinsIn : m_pinsOut;
		for (NodePinPtr pin : pinArray)
		{
			std::shared_ptr<t_pin_type> derivedPin= std::dynamic_pointer_cast<t_pin_type>(pin);
			if (derivedPin)
				return derivedPin;
		}

		return std::shared_ptr<t_pin_type>();
	}

	inline void setNodePos(const glm::vec2& nodePos) { m_nodePos= nodePos; }

	template <class t_pin_type>
	std::shared_ptr<t_pin_type> addPin(const std::string& name, eNodePinDirection direction)
	{
		NodePtr ownerNode= shared_from_this();
		std::shared_ptr<t_pin_type> pin= std::make_shared<t_pin_type>(ownerNode);
		pin->setName(name);
		pin->setDirection(direction);
		if (direction == eNodePinDirection::OUTPUT) m_pinsOut.push_back(pin);
		else if (direction == eNodePinDirection::INPUT) m_pinsOut.push_back(pin);

		// Tell the graph about the new pin
		ownerNode->getOwnerGraph()->addPin(pin);

		return pin;
	}

	bool disconnectPin(NodePinPtr pinPtr);
	void disconnectAllPins();

	virtual bool evaluateNode(NodeEvaluator& evaluator);
	virtual bool hasAnyFlowPins() const { return false; }
	virtual FlowPinPtr getOutputFlowPin() const;

	virtual std::string editorGetTitle() const { return "Node"; }
	virtual bool editorCanDelete() const { return true; }
	virtual void editorRenderNode(const NodeEditorState& editorState);
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) {}

protected:
	bool evaluateInputs(NodeEvaluator& evaluator);

	virtual void editorRenderTitle(const NodeEditorState& editorState) const;

	virtual void editorComputeNodeDimensions(NodeDimensions& outDims) const;
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const;
	virtual void editorRenderPopNodeStyle(const NodeEditorState& editorState) const;
	virtual void editorRenderInputPins(const NodeEditorState& editorState) const;
	virtual void editorRenderOutputPins(const NodeEditorState& editorState) const;

protected:
	t_node_id m_id;
	NodeGraphPtr m_ownerGraph;
	std::vector<NodePinPtr> m_pinsIn;
	std::vector<NodePinPtr> m_pinsOut;
	glm::vec2 m_nodePos;
};

class NodeFactory
{
public:
	NodeFactory()= default;
	NodeFactory(NodeGraphPtr ownerGraph);

	inline NodeConstPtr getNodeDefaultObject() const { return m_nodeDefaultObject; }
	virtual NodePtr createNode(const class NodeEditorState* editorState) const;
	
	template <class t_node_factory_class>
	static NodeFactoryPtr create(NodeGraphPtr ownerGraph)
	{
		// Create a node factory instance
		auto nodeFactory= std::make_shared<t_node_factory_class>(ownerGraph);

		// Create a single "node default object" for the factory.
		// This is used to ask questions about node without having to create one first.
		// We have to do this work outside of the NodeFactory constructor,
		// because virtual functions aren't safe to call in constructor.
		nodeFactory->m_nodeDefaultObject= nodeFactory->createNode(nullptr);

		return nodeFactory;
	}

protected:
	void autoConnectInputPin(const NodeEditorState* editorState, NodePinPtr outputPin) const;
	void autoConnectOutputPin(const NodeEditorState* editorState, NodePinPtr outputPin) const;

protected:
	NodeGraphPtr m_ownerGraph;
	NodePtr m_nodeDefaultObject;
};