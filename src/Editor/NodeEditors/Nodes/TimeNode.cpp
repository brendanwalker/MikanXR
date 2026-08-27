#include "TimeNode.h"
#include "IconsForkAwesome.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"

#include "imgui.h"

// -- TimeNode -----
bool TimeNode::evaluateNode(NodeEvaluator& evaluator)
{
	m_currentTime+= evaluator.getDeltaSeconds();

	FloatPinPtr outPin= getFirstPinOfType<FloatPin>(eNodePinDirection::OUTPUT);
	if (outPin)
	{
		outPin->setValue(m_currentTime);
	}

	return true;
}

ImVec4 TimeNode::editorGetHeaderColor() const
{
	return ImVec4(110.f / 255.f, 146.f / 255.f, 104.f / 255.f, 225.f / 255.f);
}

// -- TimeNode Factory -----
NodePtr TimeNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node= NodeFactory::createNode(editorState);
	FloatPinPtr outputPin= node->addPin<FloatPin>("time", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}

const char* TimeNode::editorGetHeaderIcon() const { return ICON_FK_CLOCK_O; }
