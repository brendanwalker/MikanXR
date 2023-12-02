#include "EventNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>

EventNode::EventNode()
	: Node()
	, m_eventNodeType(eEventNodeType::INIT)
{

}

EventNode::EventNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
	, m_eventNodeType(eEventNodeType::INIT)
{

}

void EventNode::editorRenderPushNodeStyle(NodeEditorState* editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 30, 30, 225));
}

std::string EventNode::editorGetTitle() const
{
	switch (m_eventNodeType)
	{
		case eEventNodeType::INIT:
			return "On Init";
		case eEventNodeType::FRAME:
			return "On Frame";
		default:
			return "<INVALID EVENT>";
	}
}