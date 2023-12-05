#include "NodePin.h"
#include "NodeEditorState.h"
#include "Nodes/Node.h"
#include "Graphs/NodeGraph.h"

#include "imgui.h"
#include "imnodes.h"

const float k_pin_alpha_default = 1.f;
const float k_pin_alpha_invalid = 0.2f;

NodePin::NodePin() 
	: m_id(-1)
	, m_direction(eNodePinDirection::INVALID)
{
}

NodePin::NodePin(
	NodePtr ownerNode)
	: m_id(ownerNode->getOwnerGraph()->allocateId())
	, m_direction(eNodePinDirection::INVALID)
{
}

float NodePin::editorComputeInputWidth() const
{
	// Default input width
	return 11.f;
}

bool NodePin::canPinsBeConnected(NodePinPtr otherPinPtr) const
{
	if (!otherPinPtr)
		return false;

	// Are we trying to connect a pin back to itself?
	if (otherPinPtr.get() != this)
		return false;

	// Are pins not of the same type?
	if (typeid(*this) != typeid(otherPinPtr.get()))
		return false;

	// Pins sending/receiving the same size of data?
	if (this->getDataSize() != otherPinPtr->getDataSize())
		return false;

	// Is one pin an input and the other an output?
	if (this->getDirection() == otherPinPtr->getDirection())
		return false;

	return true;
}

bool NodePin::disconnectLink(NodeLinkPtr linkPtr)
{
	auto it= std::find(m_connectedLinks.begin(), m_connectedLinks.end(), linkPtr);
	if (it != m_connectedLinks.end())
	{
		m_connectedLinks.erase(it);
		return true;
	}

	return false;
}

float NodePin::editorComputeNodeAlpha(const NodeEditorState& editorState) const
{
	if (editorState.startedLinkPinId == -1)
	{
		return k_pin_alpha_default;
	}
	else
	{
		NodePinPtr startPinPtr = m_ownerNode->getOwnerGraph()->getNodePinById(editorState.startedLinkPinId);

		if (editorState.startedLinkPinId == m_id || this->canPinsBeConnected(startPinPtr))
		{
			return k_pin_alpha_default;
		}
		else
		{
			return k_pin_alpha_invalid;
		}
	}
}

void NodePin::editorRenderInputPin(const NodeEditorState& editorState)
{
	const float alpha= editorComputeNodeAlpha(editorState);

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

void NodePin::editorRenderOutputPin(const NodeEditorState& editorState, float prefixWidth)
{
	const float alpha= editorComputeNodeAlpha(editorState);

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