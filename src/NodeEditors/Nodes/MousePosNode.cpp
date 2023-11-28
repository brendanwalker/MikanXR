#include "MousePosNode.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

MousePosNode::MousePosNode() : Node()
{}

MousePosNode::MousePosNode(NodeGraphPtr parentGraph) : Node(parentGraph)
{}

void MousePosNode::editorRender(NodeEditorState* editorState)
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(160, 160, 40, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(160, 160, 40, 225));

	ImNodes::BeginNode(m_id);

	// Title
	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text("Mouse Position");
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