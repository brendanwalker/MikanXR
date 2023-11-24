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

void EventNode::editorRender(NodeEditorState* editorState)
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 30, 30, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 30, 30, 225));

	ImNodes::BeginNode(m_id);

	// Title
	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	
	switch (m_eventNodeType)
	{
		case eEventNodeType::INIT:
			ImGui::Text("On Init");
			break;
		case eEventNodeType::FRAME:
			ImGui::Text("On Frame");
			break;
		default:
			break;
	}

	ImGui::PopStyleVar();
	ImNodes::EndNodeTitleBar();

	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	for (auto& pin : m_pinsOut)
	{
		float alpha = 1.0f;

		NodePinPtr pinPtr= m_ownerGraph->getNodePinById(editorState->startedLinkPinId);

		if (!pinPtr)
		{
			if (editorState->startedLinkPinId == pin->getId() ||
				(typeid(pinPtr.get()) == typeid(pin.get())
				 && pinPtr->getSize() == pin->getSize()
				 && pinPtr->getDirection() == eNodePinDirection::INPUT
				 && pinPtr->getOwnerNode().get() != this))
			{
				alpha = 1.0f;
			}
			else
			{
				alpha = 0.2f;
			}
		}
		else
		{
			alpha = 1.0f;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

		ImNodesPinShape pinShape = pinPtr->editorRenderBeginPin(alpha);

		ImNodes::BeginOutputAttribute(pin->getId(), pinShape);
		ImGui::Text(" ");
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(11.0f, 1.0f));
		ImNodes::EndOutputAttribute();

		pinPtr->editorRenderEndPin();

		ImGui::PopStyleColor();
	}
	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	ImNodes::EndNode();

	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}