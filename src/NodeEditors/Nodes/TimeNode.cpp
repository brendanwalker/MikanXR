#include "TimeNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

TimeNode::TimeNode() : Node()
{}

TimeNode::TimeNode(NodeGraphPtr parentGraph) : Node(parentGraph)
{}

void TimeNode::editorRenderPushNodeStyle(NodeEditorState* editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(110, 146, 104, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(110, 146, 104, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(110, 146, 104, 225));
}