#include "Node.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/NodePin.h"
#include "Pins/FlowPin.h"
#include "Logger.h"

#include "imgui.h"
#include "imnodes.h"

// -- NodeConfig -----
configuru::Config NodeConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["class_name"]= className;
	pt["id"]= id;
	writeStdValueVector<t_node_pin_id>(pt, "pins_in", pinIDsIn);
	writeStdValueVector<t_node_pin_id>(pt, "pins_out", pinIDsOut);
	writeStdArray<float, 2>(pt, "pos", pos);

	return pt;
}

void NodeConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className= pt.get_or<std::string>("class_name", "Node");
	id= pt.get_or<t_node_id>("id", -1);
	readStdValueVector<t_node_pin_id>(pt, "pins_in", pinIDsIn);
	readStdValueVector<t_node_pin_id>(pt, "pins_out", pinIDsOut);
	readStdArray<float, 2>(pt, "pos", pos);
}

// -- Node -----
Node::Node()
	: m_id(-1)
	, m_nodePos(glm::vec2(0.f))
{

}

bool Node::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	bool success= true;

	m_id= nodeConfig->id;
	m_nodePos= {nodeConfig->pos[0], nodeConfig->pos[1]};

	for (t_node_pin_id pinId : nodeConfig->pinIDsIn)
	{
		NodePinPtr pin = m_ownerGraph->getNodePinById(pinId);
		if (pin)
		{
			m_pinsIn.push_back(pin);
		}
		else
		{
			MIKAN_LOG_WARNING("Node::loadFromConfig") 
				<< "Failed to find create input pin: " << pinId 
				<< ", on node: " << getClassName();
			success= false;
		}
	}

	for (t_node_pin_id pinId : nodeConfig->pinIDsOut)
	{
		NodePinPtr pin = m_ownerGraph->getNodePinById(pinId);
		if (pin)
		{
			m_pinsOut.push_back(pin);
		}
		else
		{
			MIKAN_LOG_WARNING("Node::loadFromConfig")
				<< "Failed to find create output pin: " << pinId
				<< ", on node: " << getClassName();
			success= false;
		}
	}

	return success;
}

void Node::saveToConfig(NodeConfigPtr nodeConfig) const
{
	nodeConfig->className = getClassName();
	nodeConfig->id = m_id;
	nodeConfig->pos = {m_nodePos.x, m_nodePos.y};

	for (NodePinPtr pin : m_pinsIn)
	{
		nodeConfig->pinIDsIn.push_back(pin->getId());
	}

	for (NodePinPtr pin : m_pinsOut)
	{
		nodeConfig->pinIDsOut.push_back(pin->getId());
	}
}

void Node::setOwnerGraph(NodeGraphPtr ownerGraph) 
{ 
	m_ownerGraph = ownerGraph; 
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

		if (!m_ownerGraph->deletePinById(pinId))
		{
			MIKAN_LOG_ERROR("Node::disconnectAllPins") 
				<< "Failed to delete output pin id: " << pinId
				<< ", on node class: " << getClassName();
			break;
		}
	}

	// Delete all the input pins associated with the node
	while (m_pinsIn.size() > 0)
	{
		const t_node_pin_id pinId = m_pinsIn[0]->getId();

		if (!m_ownerGraph->deletePinById(pinId))
		{
			MIKAN_LOG_ERROR("Node::disconnectAllPins")
				<< "Failed to delete input pin id: " << pinId
				<< ", on node class: " << getClassName();
			break;
		}
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

bool Node::hasAnyConnectedPins() const
{
	for (const NodePinPtr& pin : m_pinsIn)
	{
		if (pin->hasAnyConnectedLinks())
			return true;
	}

	for (const NodePinPtr& pin : m_pinsOut)
	{
		if (pin->hasAnyConnectedLinks())
			return true;
	}

	return false;
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

void Node::editorRenderInputPins(const NodeEditorState& editorState)
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
NodeConfigPtr NodeFactory::allocateNodeConfig() const
{
	return std::make_shared<NodeConfig>();
}

NodePtr NodeFactory::allocateNode() const
{
	return std::make_shared<Node>();
}

NodePtr NodeFactory::createNode(const NodeEditorState& editorState) const
{
	NodePtr newNode= allocateNode();
	newNode->setId(editorState.nodeGraph->allocateId());

	// Assign graph to the node, which may in turn setup listener delegates to graph events
	newNode->setOwnerGraph(editorState.nodeGraph);

	return newNode;
}

void NodeFactory::autoConnectInputPin(const NodeEditorState& editorState, NodePinPtr inputPin) const
{
	assert(inputPin->getDirection() == eNodePinDirection::INPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the input pin to a compatible output pin
	if (editorState.startedLinkPinId != -1)
	{
		NodePinPtr outputPin = editorState.nodeGraph->getNodePinById(editorState.startedLinkPinId);

		if (inputPin->canPinsBeConnected(outputPin))
		{
			assert(outputPin->getDirection() == eNodePinDirection::OUTPUT);

			editorState.nodeGraph->createLink(inputPin->getId(), outputPin->getId());
		}
	}
}

void NodeFactory::autoConnectOutputPin(const NodeEditorState& editorState, NodePinPtr outputPin) const
{
	assert(outputPin->getDirection() == eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the output pin to a compatible input pin
	if (editorState.startedLinkPinId != -1)
	{
		NodePinPtr inputPin = editorState.nodeGraph->getNodePinById(editorState.startedLinkPinId);

		if (outputPin->canPinsBeConnected(inputPin))
		{
			assert(inputPin->getDirection() == eNodePinDirection::INPUT);

			editorState.nodeGraph->createLink(inputPin->getId(), outputPin->getId());
		}
	}
}