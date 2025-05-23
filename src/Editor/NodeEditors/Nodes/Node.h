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
	virtual ~Node() {}

	inline void markPendingDeletion() { m_bIsPendingDeletion= true; }
	inline bool isPendingDeletion() const { return m_bIsPendingDeletion; }

	inline bool isDefaultNode() const { return m_id == -1; }

	inline static const std::string k_nodeClassName = "Node";
	virtual std::string getClassName() const { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig);
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const;

	inline void setId(t_node_id id) { m_id = id; }
	inline int getId() const { return m_id; }

	virtual void setOwnerGraph(NodeGraphPtr ownerGraph);
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }	

	inline void setNodePos(const glm::vec2& nodePos) { m_nodePos = nodePos; }
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

	template <class t_pin_type>
	std::shared_ptr<t_pin_type> addPin(const std::string& name, eNodePinDirection direction)
	{
		NodePinPtr newPin= addPinByClassName(t_pin_type::k_pinClassName, name, direction);
		return std::static_pointer_cast<t_pin_type>(newPin);
	}
	NodePinPtr addPinByClassName(const std::string& className, const std::string& name, eNodePinDirection direction);

	bool disconnectPin(NodePinPtr pinPtr);
	void disconnectAllPins();

	virtual bool evaluateNode(NodeEvaluator& evaluator);
	virtual bool hasAnyFlowPins() const { return false; }
	virtual bool hasAnyConnectedPins() const;
	virtual FlowPinPtr getOutputFlowPin() const;

	virtual std::string editorGetTitle() const { return "Node"; }
	virtual bool editorCanDelete() const { return true; }
	virtual void editorRenderNode(const NodeEditorState& editorState);
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) {}

protected:
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) {}
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) {}

	bool evaluateInputs(NodeEvaluator& evaluator);

	virtual void editorRenderTitle(const NodeEditorState& editorState) const;
	virtual void editorComputeNodeDimensions(NodeDimensions& outDims) const;
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const;
	virtual void editorRenderPopNodeStyle(const NodeEditorState& editorState) const;
	virtual void editorRenderInputPins(const NodeEditorState& editorState);
	virtual void editorRenderOutputPins(const NodeEditorState& editorState) const;

protected:
	t_node_id m_id;
	NodeGraphPtr m_ownerGraph;
	std::vector<NodePinPtr> m_pinsIn;
	std::vector<NodePinPtr> m_pinsOut;
	glm::vec2 m_nodePos;
	bool m_bIsPendingDeletion;

	// NodePin calls onLinkConnected/ onLinkDisconnected
	friend class NodePin;
};

class NodeFactory
{
public:
	NodeFactory()= default;

	inline NodeConstPtr getNodeDefaultObject() const { return m_nodeDefaultObject; }
	inline std::string getNodeClassName() const { return m_nodeDefaultObject->getClassName(); }

	virtual NodeConfigPtr allocateNodeConfig() const;
	virtual NodePtr allocateNode() const;
	virtual NodePtr createNode(const NodeEditorState& editorState) const;

	virtual bool editorCanCreate() const { return getNodeClassName() != Node::k_nodeClassName; }

	template <class t_node_factory_class>
	static NodeFactoryPtr createFactory()
	{
		// Create a node factory instance
		auto nodeFactory= std::make_shared<t_node_factory_class>();

		// Create a single "node default object" for the factory.
		// This is used to ask questions about node without having to create one first.
		// We have to do this work outside of the NodeFactory constructor,
		// because virtual functions aren't safe to call in constructor.
		nodeFactory->m_nodeDefaultObject= nodeFactory->allocateNode();

		return nodeFactory;
	}

protected:
	void autoConnectInputPin(const NodeEditorState& editorState, NodePinPtr outputPin) const;
	void autoConnectOutputPin(const NodeEditorState& editorState, NodePinPtr outputPin) const;

protected:
	NodePtr m_nodeDefaultObject;
};

template <class t_node_class, class t_node_config_class>
class TypedNodeFactory : public NodeFactory
{
public:
	TypedNodeFactory() = default;

	virtual NodeConfigPtr allocateNodeConfig() const override
	{
		return std::make_shared<t_node_config_class>();
	}

	virtual NodePtr allocateNode() const override
	{
		return std::make_shared<t_node_class>();
	}
};