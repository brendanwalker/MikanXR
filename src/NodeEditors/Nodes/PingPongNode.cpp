#include "PingPongNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

PingPongNode::PingPongNode()
	: Node()
{
}

PingPongNode::PingPongNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
{
}

void PingPongNode::editorRenderPushNodeStyle(NodeEditorState* editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(83, 124, 153, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(83, 124, 153, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(83, 124, 153, 225));
}