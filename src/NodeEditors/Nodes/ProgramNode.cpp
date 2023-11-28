#include "ProgramNode.h"
#include "GlProgram.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>
#include <GL/glew.h>

ProgramNode::ProgramNode()
	: Node()
	, m_attachmentsPinsStartId(1)
	, m_dispatchType(ProgramDispatchType::ARRAY)
	, m_drawMode(GL_POINTS)
{
	m_dispatchSize[3] = {};
}

ProgramNode::ProgramNode(NodeGraphPtr parentGraph)
	: Node(parentGraph)
	, m_attachmentsPinsStartId(1)
	, m_dispatchType(ProgramDispatchType::ARRAY)
	, m_drawMode(GL_POINTS)
{

}

void ProgramNode::editorRender(NodeEditorState* editorState)
{
	const std::string& programName= m_target->getProgramCode().getProgramName();

	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(85, 85, 85, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(85, 85, 85, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(85, 85, 85, 225));

	ImNodes::BeginNode(m_id);

	// Title
	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text(programName.c_str());
	ImGui::PopStyleVar();
	ImNodes::EndNodeTitleBar();

	// Compute node width
	float nodeWidth = ImGui::CalcTextSize(programName.c_str()).x;
	float nodeInWidth = 0.0f;
	float nodeOutWidth = 0.0f;

	for (auto& pin : m_pinsIn)
	{
		float textWidth = ImGui::CalcTextSize(pin->getName().c_str()).x + 11.0f;
		const float inputWidth = pin->editorComputeInputWidth();

		nodeInWidth = std::max(nodeInWidth, std::max(textWidth, inputWidth));
	}

	for (auto& pin : m_pinsOut)
	{
		nodeOutWidth = std::max(nodeOutWidth, ImGui::CalcTextSize(pin->getName().c_str()).x + 11.0f);
	}

	nodeWidth = std::max(nodeWidth, nodeInWidth + nodeOutWidth);

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
		float prefixWidth = nodeWidth - nodeInWidth - ImGui::CalcTextSize(pin->getName().c_str()).x;

		pin->editorRenderOutputPin(editorState, prefixWidth);
	}
	ImGui::EndGroup();
	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}