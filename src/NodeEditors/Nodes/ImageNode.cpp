#include "ImageNode.h"
#include "GlCommon.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

ImageNode::ImageNode()
	: Node()
{}

ImageNode::ImageNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
{}

ImageNode::~ImageNode()
{
	if (m_texture != -1)
		glDeleteTextures(1, &m_texture);
}

void ImageNode::editorRenderPushNodeStyle(NodeEditorState* editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(118, 32, 140, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(118, 32, 140, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(118, 32, 140, 225));
}