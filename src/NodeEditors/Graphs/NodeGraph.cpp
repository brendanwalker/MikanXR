#include "NodeGraph.h"
#include "Nodes/Node.h"
#include "Pins/NodeLink.h"
#include "Pins/NodePin.h"

#include "imnodes.h"

NodeGraph::NodeGraph()
{

}

NodeGraph::~NodeGraph()
{

}

NodePtr NodeGraph::getNodeById(int id)
{
	auto it = m_Nodes.find(id);
	if (it != m_Nodes.end())
	{
		return it->second;
	}

	return NodePtr();
}

NodePinPtr NodeGraph::getNodePinById(int id)
{
	auto it = m_Pins.find(id);
	if (it != m_Pins.end())
	{
		return it->second;
	}

	return NodePinPtr();
}

NodeLinkPtr NodeGraph::getNodeLinkById(int id)
{
	auto it = m_Links.find(id);
	if (it != m_Links.end())
	{
		return it->second;
	}

	return NodeLinkPtr();
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