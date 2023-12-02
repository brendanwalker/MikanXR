#include "BlockNode.h"
#include "GlCommon.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

BlockNode::BlockNode()
	: Node()
{
}

BlockNode::BlockNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
{
}

BlockNode::~BlockNode()
{
	if (m_ubo != -1)
		glDeleteBuffers(1, &m_ubo);
	if (m_ssbo != -1)
		glDeleteBuffers(1, &m_ssbo);
}

void BlockNode::editorRenderPushNodeStyle(NodeEditorState* editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(83, 124, 153, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(83, 124, 153, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(83, 124, 153, 225));
}