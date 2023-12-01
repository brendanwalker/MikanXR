#include "Node.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"

#include "imgui.h"
#include "imnodes.h"

Node::Node()
	: m_id(-1)
	, m_nodePos(glm::vec2(0.f))
{

}

Node::Node(NodeGraphPtr ownerGraph)
	: m_id(ownerGraph->allocateId())
	, m_nodePos(glm::vec2(0.f))
{
	
}

ImU32 Node::editorGetTitleBarColor() const
{ 
	return IM_COL32(85, 85, 85, 255); 
}

ImU32 Node::editorGetTitleBarHoveredColor() const
{
	return IM_COL32(85, 85, 85, 255);
}

ImU32 Node::editorGetTitleBarSelectedColor() const
{
	return IM_COL32(85, 85, 85, 255);
}

void Node::editorRender(NodeEditorState* editorState)
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, editorGetTitleBarColor());
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, editorGetTitleBarHoveredColor());
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, editorGetTitleBarSelectedColor());

	ImNodes::BeginNode(m_id);

	// Title
	editorRenderTitle(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	// Inputs
	editorRenderInputPins(editorState);

	// Outputs
	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}

void Node::editorComputeNodeDimensions(NodeDimensions& outDims) const
{
	const std::string titleString= editorGetTitle();
	outDims.titleWidth = ImGui::CalcTextSize(titleString.c_str()).x;
	outDims.totalNodeWidth= outDims.titleWidth;
	outDims.inputColomnWidth = 0.0f;
	outDims.outputColomnWidth = 0.0f;

	for (auto& pin : m_pinsIn)
	{
		float textWidth = ImGui::CalcTextSize(pin->getName().c_str()).x + 11.0f;
		const float inputWidth = pin->editorComputeInputWidth();

		outDims.inputColomnWidth = 
			std::max(
				outDims.inputColomnWidth, 
				std::max(textWidth, inputWidth));
	}

	for (auto& pin : m_pinsOut)
	{
		outDims.outputColomnWidth = 
			std::max(
				outDims.outputColomnWidth, 
				ImGui::CalcTextSize(pin->getName().c_str()).x + 11.0f);
	}

	outDims.totalNodeWidth = 
		std::max(
			outDims.totalNodeWidth, 
			outDims.inputColomnWidth + outDims.outputColomnWidth);
}

void Node::editorRenderTitle(NodeEditorState* editorState) const
{
	const std::string titleString= editorGetTitle();

	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text(titleString.c_str());
	ImGui::PopStyleVar();
	ImNodes::EndNodeTitleBar();
}

void Node::editorRenderInputPins(NodeEditorState* editorState) const
{
	if (m_pinsIn.size() > 0)
	{
		ImGui::BeginGroup();
		for (auto& pin : m_pinsIn)
		{
			pin->editorRenderInputPin(editorState);
		}
		ImGui::EndGroup();
		ImGui::SameLine();
	}
}

void Node::editorRenderOutputPins(
	NodeEditorState* editorState) const
{
	if (m_pinsOut.size() == 0)
		return;

	NodeDimensions nodeDims= {};
	editorComputeNodeDimensions(nodeDims);

	ImGui::BeginGroup();
	for (auto& pin : m_pinsOut)
	{
		const float prefixWidth = 
			nodeDims.totalNodeWidth 
			- nodeDims.inputColomnWidth
			- ImGui::CalcTextSize(pin->getName().c_str()).x;

		pin->editorRenderOutputPin(editorState, prefixWidth);
	}
	ImGui::EndGroup();
}