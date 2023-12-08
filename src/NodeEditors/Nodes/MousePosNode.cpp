#include "MousePosNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FloatPin.h"
#include "IGlWindow.h"
#include "GlViewport.h"

#include "imgui.h"
#include "imnodes.h"

MousePosNode::MousePosNode() : Node()
{}

MousePosNode::MousePosNode(NodeGraphPtr parentGraph) : Node(parentGraph)
{}

void MousePosNode::evaluateNode(NodeEvaluator& evaluator)
{
	Float2PinPtr outPin= getFirstPinOfType<Float2Pin>(eNodePinDirection::OUTPUT);

	glm::vec2 pixelPos;
	auto viewport= evaluator.getCurrentWindow()->getRenderingViewport();
	if (viewport && viewport->getCursorViewportPixelPos(pixelPos))
	{
		auto viewportSize= viewport->getViewportSize();

		outPin->setValue({pixelPos.x / (float)viewportSize.x, pixelPos.y / (float)viewportSize.y});
	}
}

void MousePosNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(160, 160, 40, 225));
}