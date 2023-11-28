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

void ImageNode::editorRender(NodeEditorState* editorState)
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(118, 32, 140, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(118, 32, 140, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(118, 32, 140, 225));

	ImNodes::BeginNode(m_id);

	// Title
	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text("Image");
	ImGui::PopStyleVar();
	ImNodes::EndNodeTitleBar();

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	// Inputs
	ImGui::BeginGroup();
	for (auto& pin : m_pinsIn)
	{
		pin->editorRenderInputPin(editorState);
	}
	ImGui::EndGroup();
	ImGui::SameLine();

	// Outputs
	ImGui::BeginGroup();
	for (auto& pin : m_pinsOut)
	{
		pin->editorRenderOutputPin(editorState);
	}
	ImGui::EndGroup();
	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}