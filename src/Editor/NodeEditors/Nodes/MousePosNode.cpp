#include "MousePosNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"
#include "IMkGraphicsContext.h"
#include "MikanViewport.h"

#include "imgui.h"
#include "imnodes.h"

bool MousePosNode::evaluateNode(NodeEvaluator& evaluator)
{
	Float2PinPtr outPin= getFirstPinOfType<Float2Pin>(eNodePinDirection::OUTPUT);

	glm::vec2 pixelPos;
	auto graphicsContext= evaluator.getCurrentGraphicsContext();
	auto viewport=
		std::static_pointer_cast<MikanViewport>(
			graphicsContext->getRenderingViewport());
	if (viewport && viewport->getCursorViewportPixelPos(pixelPos))
	{
		auto viewportSize= viewport->getViewportSize();

		outPin->setValue({pixelPos.x / (float)viewportSize.x, pixelPos.y / (float)viewportSize.y});
	}

	return true;
}

std::shared_ptr<MkNodesScopedColorStyle> MousePosNode::editorRenderMakeNodeStyle(const NodeEditorState& editorState) const
{
	auto style= std::make_shared<MkNodesScopedColorStyle>();
	style->push(ImNodesCol_TitleBar, IM_COL32(160, 160, 40, 225))
		.push(ImNodesCol_TitleBarHovered, IM_COL32(160, 160, 40, 225))
		.push(ImNodesCol_TitleBarSelected, IM_COL32(160, 160, 40, 225));
	return style;
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