#include "MousePosNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"
#include "IMkGraphicsContext.h"
#include "MikanViewport.h"

#include "imgui.h"

bool MousePosNode::evaluateNode(NodeEvaluator& evaluator)
{
	Float2PinPtr outPin= getFirstPinOfType<Float2Pin>(eNodePinDirection::OUTPUT);

	glm::vec2 pixelPos;
	auto graphicsContext= evaluator.getCurrentGraphicsContext();
	auto viewport= std::static_pointer_cast<MikanViewport>(graphicsContext->getRenderingViewport());
	if (viewport && viewport->getCursorViewportPixelPos(pixelPos))
	{
		auto viewportSize= viewport->getViewportSize();

		outPin->setValue({pixelPos.x / (float)viewportSize.x, pixelPos.y / (float)viewportSize.y});
	}

	return true;
}

ImVec4 MousePosNode::editorGetHeaderColor() const
{
	return ImVec4(160.f / 255.f, 160.f / 255.f, 40.f / 255.f, 225.f / 255.f);
}

// -- MousePosNode Factory -----
NodePtr MousePosNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and pins
	NodePtr node= NodeFactory::createNode(editorState);
	FloatPinPtr outputPin= node->addPin<FloatPin>("mousePos", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	autoConnectOutputPin(editorState, outputPin);

	return node;
}