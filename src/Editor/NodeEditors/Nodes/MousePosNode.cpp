#include "MousePosNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"
#include "ISdlMkWindow.h"
#include "MikanViewport.h"

#include "imgui.h"
#include "imnodes.h"

bool MousePosNode::evaluateNode(NodeEvaluator& evaluator)
{
	Float2PinPtr outPin= getFirstPinOfType<Float2Pin>(eNodePinDirection::OUTPUT);

	glm::vec2 pixelPos;
	auto viewport= 
		std::static_pointer_cast<MikanViewport>(
			evaluator.getCurrentWindow()->getRenderingViewport());
	if (viewport && viewport->getCursorViewportPixelPos(pixelPos))
	{
		auto viewportSize= viewport->getViewportSize();

		outPin->setValue({pixelPos.x / (float)viewportSize.x, pixelPos.y / (float)viewportSize.y});
	}

	return true;
}

void MousePosNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(160, 160, 40, 225));
}

// -- MousePosNode Factory -----
NodePtr MousePosNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node = NodeFactory::createNode(editorState);
	FloatPinPtr outputPin = node->addPin<FloatPin>("mousePos", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}