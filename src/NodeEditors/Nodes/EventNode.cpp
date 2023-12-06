#include "EventNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>

EventNode::EventNode()
	: Node()
{

}

EventNode::EventNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
{

}

void EventNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 30, 30, 225));
}

std::string EventNode::editorGetTitle() const
{
	return m_eventName;
}