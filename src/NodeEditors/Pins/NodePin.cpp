#include "NodePin.h"
#include "NodeEditorState.h"
#include "Nodes/Node.h"
#include "Graphs/NodeGraph.h"

#include "imgui.h"
#include "imnodes.h"

NodePin::NodePin() 
	: m_id(-1)
	, m_direction(eNodePinDirection::INPUT)
{
}

NodePin::NodePin(NodePtr ownerNode)
	: m_id(ownerNode->getOwnerGraph()->allocateId())
	, m_size(1)
	, m_direction(eNodePinDirection::INPUT)
{
}

float NodePin::editorComputeInputWidth() const
{
	// Default input width
	return 11.f;
}

void NodePin::editorRenderInputPin(NodeEditorState* editorState)
{
	float alpha = 0.2f;
	if (editorState->startedLinkPinId == -1)
	{
		alpha = 1.0f;
	}
	else
	{
		NodePinPtr startPinPtr = m_ownerNode->getOwnerGraph()->getNodePinById(editorState->startedLinkPinId);

		if (editorState->startedLinkPinId == m_id ||
			(typeid(startPinPtr.get()) == typeid(*this)
			 && startPinPtr->getDirection() == eNodePinDirection::OUTPUT
			 && startPinPtr->getOwnerNode() != m_ownerNode))
		{
			if (startPinPtr->getSize() == this->getSize())
			{
				alpha = 1.0f;
			}
			// TODO
			/*
			else if ((m_Pins[m_StartedLinkPinId]->ownerNode->type == EditorNodeType::PINGPONG
					  && m_Pins[m_StartedLinkPinId]->size == 0) ||
					 (node->type == EditorNodeType::PINGPONG && pin->size == 0))
			{
				alpha = 1.0f;
			}
			*/
		}
	}

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

	ImNodesPinShape pinShape = editorRenderBeginPin(alpha);

	ImNodes::BeginInputAttribute(m_id, pinShape);
	ImGui::Dummy(ImVec2(11.0f, 1.0f));
	ImGui::SameLine();
	ImGui::Text(m_name.c_str());
	editorRenderInputTextEntry(editorState);
	ImNodes::EndInputAttribute();

	editorRenderEndPin();

	ImGui::PopStyleColor();
}

void NodePin::editorRenderOutputPin(NodeEditorState* editorState, float prefixWidth)
{
	float alpha = 0.2f;
	if (editorState->startedLinkPinId == -1)
	{
		alpha = 1.0f;
	}
	else
	{
		NodePinPtr startPinPtr = m_ownerNode->getOwnerGraph()->getNodePinById(editorState->startedLinkPinId);

		if (editorState->startedLinkPinId == m_id ||
			(typeid(startPinPtr.get()) == typeid(*this)
			 && startPinPtr->getDirection() == eNodePinDirection::INPUT
			 && startPinPtr->getOwnerNode() != m_ownerNode))
		{
			if (startPinPtr->getSize() == this->getSize())
			{
				alpha = 1.0f;
			}
			// TODO
			/*
			else if ((m_Pins[m_StartedLinkPinId]->ownerNode->type == EditorNodeType::PINGPONG
					  && m_Pins[m_StartedLinkPinId]->size == 0) ||
					 (node->type == EditorNodeType::PINGPONG && pin->size == 0))
			{
				alpha = 1.0f;
			}
			*/
		}
	}

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

	ImNodesPinShape pinShape = editorRenderBeginPin(alpha);

	ImNodes::BeginOutputAttribute(m_id, pinShape);	
	if (prefixWidth > 0.f)
	{
		ImGui::Dummy(ImVec2(prefixWidth, 1.0f));
		ImGui::SameLine();
	}
	ImGui::Text(m_name.c_str());
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(11.0f, 1.0f));
	ImNodes::EndOutputAttribute();

	editorRenderEndPin();

	ImGui::PopStyleColor();
}

ImNodesPinShape NodePin::editorRenderBeginPin(float alpha)
{
	ImNodesPinShape pinShape = ImNodesPinShape_Triangle;

	if (m_connectedLinks.size() > 0)
		pinShape = ImNodesPinShape_CircleFilled;
	else
		pinShape = ImNodesPinShape_Circle;

	ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(252, 200, 35, alpha * 255));
	ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(255, 217, 140, alpha * 255));

	return pinShape;	
}

void NodePin::editorRenderEndPin()
{
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}

void NodePin::editorRenderBeginLink(float alpha)
{
	ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(252, 200, 35, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(255, 217, 140, alpha));
	ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(255, 217, 140, 255));
}

void NodePin::editorRenderEndLink()
{
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
	ImNodes::PopColorStyle();
}

ImU32 NodePin::editorGetLinkStyleColor() const
{
	return IM_COL32(252, 200, 35, 255);
}