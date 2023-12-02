#include "MousePosNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

MousePosNode::MousePosNode() : Node()
{}

MousePosNode::MousePosNode(NodeGraphPtr parentGraph) : Node(parentGraph)
{}

void MousePosNode::editorRenderPushNodeStyle(NodeEditorState* editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(160, 160, 40, 225));
}