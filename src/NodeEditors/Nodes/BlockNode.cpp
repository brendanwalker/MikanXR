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

void BlockNode::editorRender(NodeEditorState* editorState)
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(83, 124, 153, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(83, 124, 153, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(83, 124, 153, 225));

	ImNodes::BeginNode(m_id);
	// Title
	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text("Block");
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