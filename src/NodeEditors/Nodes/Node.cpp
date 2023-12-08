#include "Node.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FlowPin.h"

#include "imgui.h"
#include "imnodes.h"

// -- Node -----
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

bool Node::disconnectPin(NodePinPtr pinPtr)
{
	if (pinPtr->getDirection() == eNodePinDirection::INPUT)
	{
		auto it = std::find(m_pinsIn.begin(), m_pinsIn.end(), pinPtr);
		if (it != m_pinsIn.end())
		{
			m_pinsIn.erase(it);
			return true;
		}
	}
	else if (pinPtr->getDirection() == eNodePinDirection::OUTPUT)
	{
		auto it = std::find(m_pinsOut.begin(), m_pinsOut.end(), pinPtr);
		if (it != m_pinsOut.end())
		{
			m_pinsOut.erase(it);
			return true;
		}
	}

	return false;
}

void Node::disconnectAllPins()
{	
	// Delete all the output pins associated with the node
	while (m_pinsOut.size() > 0)
	{
		const t_node_pin_id pinId = m_pinsOut[0]->getId();

		m_ownerGraph->deletePinById(pinId);
	}

	// Delete all the input pins associated with the node
	while (m_pinsIn.size() > 0)
	{
		const t_node_pin_id pinId = m_pinsIn[0]->getId();

		m_ownerGraph->deletePinById(pinId);
	}
}

bool Node::evaluateNode(NodeEvaluator& evaluator) 
{
	evaluator.setLastErrorCode(eNodeEvaluationErrorCode::invalidNode);
	evaluator.setLastErrorMessage("Node missing evaluateNode implementation");

	return false;
}

bool Node::evaluateInputs(NodeEvaluator& evaluator)
{
	for (auto inputPin : m_pinsIn)
	{
		assert(inputPin->getDirection() == eNodePinDirection::INPUT);

		// Don't consider flow pins
		// (the control evaluation order, not value propagation)
		if (std::dynamic_pointer_cast<FlowPin>(inputPin))
			continue;

		// Get the output pin that this input pin feed by
		NodePinPtr outputSourcePin = inputPin->getConnectedSourcePin();
		if (!outputSourcePin)
		{
			evaluator.setLastErrorCode(eNodeEvaluationErrorCode::missingInput);
			evaluator.setLastErrorMessage("pin missing input");
			return false;
		}

		// Recurse into node that owns the output and evaluate it's inputs ...
		// ... unless the source node has flow pins, 
		// which means the source need should have already been evaluated
		NodePtr sourceNode= outputSourcePin->getOwnerNode();
		if (!sourceNode->hasAnyFlowPins())
		{
			// Recursively evaluate the inputs for the source node (if any)
			if (!sourceNode->evaluateInputs(evaluator))
				return false;

			// Then evaluate the node to update its output pins
			if (!sourceNode->evaluateNode(evaluator))
				return false;
		}

		// Update the input pin now that output it's connected to is updated
		inputPin->copyValueFromSourcePin();
	}

	return true;
}

FlowPinPtr Node::getOutputFlowPin() const 
{ 
	return FlowPinPtr(); 
}

void Node::editorRenderNode(const NodeEditorState& editorState)
{
	editorRenderPushNodeStyle(editorState);

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

	editorRenderPopNodeStyle(editorState);
}

void Node::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(85, 85, 85, 255));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(85, 85, 85, 255));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(85, 85, 85, 255));
}

void Node::editorRenderPopNodeStyle(const NodeEditorState& editorState) const
{
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

void Node::editorRenderTitle(const NodeEditorState& editorState) const
{
	const std::string titleString= editorGetTitle();

	ImNodes::BeginNodeTitleBar();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::Text(titleString.c_str());
	ImGui::PopStyleVar();
	ImNodes::EndNodeTitleBar();
}

void Node::editorRenderInputPins(const NodeEditorState& editorState) const
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

void Node::editorRenderOutputPins(const NodeEditorState& editorState) const
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

// -- NodeFactory -----
NodeFactory::NodeFactory(NodeGraphPtr ownerGraph)
	: m_ownerGraph(ownerGraph)
{
}

NodePtr NodeFactory::createNode(const NodeEditorState* editorState) const
{
	return std::make_shared<Node>(m_ownerGraph);
}