#include "TimeNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"

#include "imgui.h"
#include "imnodes.h"

// -- TimeNode -----
bool TimeNode::evaluateNode(NodeEvaluator& evaluator)
{
	m_currentTime+= evaluator.getDeltaSeconds();

	FloatPinPtr outPin = getFirstPinOfType<FloatPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(m_currentTime);
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
NodePtr TimeNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	FloatPinPtr outputPin= node->addPin<FloatPin>("time", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}