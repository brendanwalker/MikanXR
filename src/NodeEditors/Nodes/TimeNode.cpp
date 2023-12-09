#include "TimeNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"

#include "imgui.h"
#include "imnodes.h"

// -- TimeNode -----
TimeNode::TimeNode() : Node()
{}

TimeNode::TimeNode(NodeGraphPtr parentGraph) : Node(parentGraph)
{}

bool TimeNode::evaluateNode(NodeEvaluator& evaluator)
{
	FloatPinPtr outPin = getFirstPinOfType<FloatPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(getOwnerGraph()->getTimeInSeconds());
	}

	return true;
}

void TimeNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(110, 146, 104, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(110, 146, 104, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(110, 146, 104, 225));
}

// -- TimeNode Factory -----
NodePtr TimeNodeFactory::createNode(const NodeEditorState* editorState) const
{
	// Create the node and pins
	TimeNodePtr node = std::make_shared<TimeNode>();
	FloatPinPtr outputPin= node->addPin<FloatPin>("time", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}