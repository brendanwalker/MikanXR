#include "MkCanvasScopedNode.h"
#include "MkCanvasWidgets.h"

#include "imgui.h"
#include "imgui_node_editor.h"

namespace ed= ax::NodeEditor;

MkCanvasScopedNode::MkCanvasScopedNode(int nodeId, const ImVec4& headerColor)
	: m_nodeId(MkCanvas::toCanvasId(nodeId))
	, m_headerColor(ImColor(headerColor))
{
	ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8.f, 4.f, 8.f, 8.f));
	ed::BeginNode(m_nodeId);
	ImGui::PushID(m_nodeId);
}

MkCanvasScopedNode::~MkCanvasScopedNode()
{
	ed::EndNode();

	// The node rect is the last submitted item; paint the header band across
	// its full width now that the final size is known (blueprints-example
	// technique, minus the noise texture)
	if (m_bHasHeader && ImGui::IsItemVisible())
	{
		const ImVec2 nodeMin= ImGui::GetItemRectMin();
		const ImVec2 nodeMax= ImGui::GetItemRectMax();
		const float halfBorder= ed::GetStyle().NodeBorderWidth * 0.5f;
		const float rounding= ed::GetStyle().NodeRounding;

		ImDrawList* drawList= ed::GetNodeBackgroundDrawList(m_nodeId);
		const ImVec2 headerBandMin(nodeMin.x + halfBorder, nodeMin.y + halfBorder);
		const ImVec2 headerBandMax(nodeMax.x - halfBorder, m_headerMax.y + 2.f);

		if (headerBandMax.y > headerBandMin.y)
		{
			drawList->AddRectFilled(headerBandMin, headerBandMax, m_headerColor, rounding, ImDrawFlags_RoundCornersTop);

			// Thin separator between the header band and the node body
			const int alpha= (int)(255 * ImGui::GetStyle().Alpha);
			drawList->AddLine(ImVec2(headerBandMin.x, headerBandMax.y - 0.5f),
							  ImVec2(headerBandMax.x, headerBandMax.y - 0.5f),
							  IM_COL32(255, 255, 255, 32 * alpha / 255), 1.f);
		}
	}

	ImGui::PopID();
	ed::PopStyleVar();
}

void MkCanvasScopedNode::beginHeader() { ImGui::BeginGroup(); }

void MkCanvasScopedNode::endHeader()
{
	ImGui::EndGroup();

	m_headerMin= ImGui::GetItemRectMin();
	m_headerMax= ImGui::GetItemRectMax();
	m_bHasHeader= true;

	// Breathing room between the header band and the first content row
	ImGui::Dummy(ImVec2(0.f, 4.f));
}
