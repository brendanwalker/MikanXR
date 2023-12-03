#include "NodeGraph.h"
#include "Nodes/Node.h"
#include "Pins/NodeLink.h"
#include "Pins/NodePin.h"
#include "NodeEditorState.h"

#include "imnodes.h"

NodeGraph::NodeGraph()
{

}

NodeGraph::~NodeGraph()
{

}

NodePtr NodeGraph::getNodeById(t_node_id id) const
{
	auto it = m_Nodes.find(id);
	if (it != m_Nodes.end())
	{
		return it->second;
	}

	return NodePtr();
}

NodePinPtr NodeGraph::getNodePinById(t_node_pin_id id) const
{
	auto it = m_Pins.find(id);
	if (it != m_Pins.end())
	{
		return it->second;
	}

	return NodePinPtr();
}

NodeLinkPtr NodeGraph::getNodeLinkById(t_node_link_id id) const
{
	auto it = m_Links.find(id);
	if (it != m_Links.end())
	{
		return it->second;
	}

	return NodeLinkPtr();
}

bool NodeGraph::deleteNodeById(t_node_id id)
{
	auto it = m_Nodes.find(id);
	if (it != m_Nodes.end())
	{
		NodePtr node= it->second;

		// Delete all pins and associated links from this node
		node->disconnectAllPins();
		
		if (OnNodeDeleted)
			OnNodeDeleted(id);

		// Erase the Node
		m_Nodes.erase(it);

		return true;
	}

	return false;
}

bool NodeGraph::deletePinById(t_node_pin_id id)
{
	auto it = m_Pins.find(id);
	if (it != m_Pins.end())
	{
		NodePinPtr pin = it->second;

		// Delete all link associated with this pin
		// (notify editor dependent links are going away first)
		auto& links= pin->getConnectedLinks();
		while (links.size() > 0)
		{
			const t_node_link_id linkId= links[0]->getId();

			deleteLinkById(linkId);
		}

		// Let the editor know the pin is about to be deleted
		if (OnPinDeleted)
			OnPinDeleted(id);

		// Remove the Pin from the owning Node
		NodePtr ownerNode= pin->getOwnerNode();
		if (ownerNode)
		{
			ownerNode->disconnectPin(pin);
		}

		// Erase the Pin
		m_Pins.erase(it);

		return true;
	}

	return false;
}


bool NodeGraph::deleteLinkById(t_node_link_id id)
{
	auto it = m_Links.find(id);
	if (it != m_Links.end())
	{
		NodeLinkPtr link= it->second;

		// Let the editor know the link is about to be deleted
		if (OnLinkDeleted)
			OnLinkDeleted(id);

		// Remove link attachments from start and end pins
		NodePinPtr startPin= link->getStartPin();
		assert(startPin);
		NodePinPtr endPin= link->getEndPin();
		assert(endPin);

		startPin->disconnectLink(link);
		endPin->disconnectLink(link);

		// Free the link
		m_Links.erase(it);

		return true;
	}

	return false;
}

std::vector<NodeFactoryPtr> NodeGraph::editorGetValidNodeFactories(const NodeEditorState& editorState) const
{
	std::vector<NodeFactoryPtr> validFactories;

	if (editorState.startedLinkPinId != -1)
	{
		NodePinPtr sourcePin= getNodePinById(editorState.startedLinkPinId);

		for (NodeFactoryPtr factory : m_nodeFactories)
		{
			NodeConstPtr nodeDefinition= factory->getNodeDefinition();
			bool bIsValidFactory= false;

			if (sourcePin->getDirection() == eNodePinDirection::INPUT)
			{
				for (NodePinPtr targetPin : nodeDefinition->getOutputPins())
				{
					if (targetPin->canPinsBeConnected(sourcePin))
					{
						bIsValidFactory = true;
						break;
					}
				}
			}
			else if (sourcePin->getDirection() == eNodePinDirection::OUTPUT)
			{
				for (NodePinPtr targetPin : nodeDefinition->getInputPins())
				{
					if (targetPin->canPinsBeConnected(sourcePin))
					{
						bIsValidFactory = true;
						break;
					}
				}
			}

			if (bIsValidFactory)
			{
				validFactories.push_back(factory);
			}
		}
	}
	else
	{
		// Return all available factories
		validFactories= m_nodeFactories;
	}

	return validFactories;
}

void NodeGraph::editorRender(class NodeEditorState* editorState)
{
	// Nodes rendering
	for (auto it= m_Nodes.begin(); it != m_Nodes.end(); ++it)
	{
		NodePtr node= it->second;

		if (ImNodes::IsNodeSelected(node->getId()))
		{
			ImNodes::PushStyleVar(ImNodesStyleVar_NodeBorderThickness, 2.6f);
			ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(220, 140, 0, 255));
		}
		else
		{
			ImNodes::PushStyleVar(ImNodesStyleVar_NodeBorderThickness, 2.0f);
			ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(24, 24, 24, 255));
		}

		node->editorRender(editorState);

		const ImVec2 nodePos = ImNodes::GetNodeScreenSpacePos(node->getId());
		node->setNodePos({nodePos.x, nodePos.y});

		ImNodes::PopColorStyle();
		ImNodes::PopStyleVar();
	}

	// Links Rendering
	for (auto it= m_Links.begin(); it != m_Links.end(); ++it)
	{
		NodeLinkPtr link = it->second;

		link->editorRender(editorState);
	}
}

int NodeGraph::allocateId()
{
	int newId = m_nextId;
	m_nextId++;
	return newId;
}