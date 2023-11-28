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

void TimeNode::editorRender(NodeEditorState* editorState)
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(110, 146, 104, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(110, 146, 104, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(110, 146, 104, 225));

	ImNodes::BeginNode(m_id);

	// Title
	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text("Time");
	ImGui::PopStyleVar();
	ImNodes::EndNodeTitleBar();

	ImGui::Dummy(ImVec2(1.0f, 0.5f));
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