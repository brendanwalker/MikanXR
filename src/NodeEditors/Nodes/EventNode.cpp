#include "EventNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/FlowPin.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>

EventNode::EventNode()
	: Node()
{

}

EventNode::EventNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
{

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
	return m_eventName;
}