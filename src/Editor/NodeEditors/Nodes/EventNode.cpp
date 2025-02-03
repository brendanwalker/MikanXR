#include "EventNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/FlowPin.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>

// -- EventNodeConfig -----

configuru::Config EventNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["event_name"] = eventName;

	return pt;
}

void EventNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	eventName = pt.get_or<std::string>("event_name", "");
}

// -- EventNode -----
bool EventNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto eventNodeConfig= std::static_pointer_cast<const EventNodeConfig>(nodeConfig);

		m_eventName= eventNodeConfig->eventName;
		return true;
	}

	return false;
}

void EventNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto eventNodeConfig= std::static_pointer_cast<EventNodeConfig>(nodeConfig);

	eventNodeConfig->eventName= m_eventName;

	Node::saveToConfig(nodeConfig);
}

bool EventNode::evaluateNode(NodeEvaluator& evaluator)
{
	// TODO: Push event parameters to output pins
	return true;
}

FlowPinPtr EventNode::getOutputFlowPin() const
{
	return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT);
}

void EventNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 30, 30, 225));
}

std::string EventNode::editorGetTitle() const
{
	return !m_eventName.empty() ? m_eventName : k_nodeClassName;
}

// -- EventNode Factory -----
NodePtr EventNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	FlowPinPtr outputPin = node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}