#include "NodeLink.h"
#include "NodePin.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"

#include "imnodes.h"

// -- NodeConfig -----
configuru::Config NodeLinkConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["id"] = id;
	pt["start_pin_id"] = start_pin_id;
	pt["end_pin_id"] = end_pin_id;

	return pt;
}

void NodeLinkConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	id = pt.get_or<t_node_id>("id", -1);
	start_pin_id = pt.get_or<t_node_id>("start_pin_id", -1);
	end_pin_id = pt.get_or<t_node_id>("end_pin_id", -1);
}

//-- NodeLink -----
NodeLink::NodeLink()
	: m_id(-1)
{
}

NodeLink::NodeLink(NodeGraphPtr ownerGraph)
	: m_id(ownerGraph->allocateId())
	, m_ownerGraph(ownerGraph)
{
}

bool NodeLink::loadFromConfig(const NodeLinkConfig& config)
{
	m_id = config.id;
	
	m_startPin= getOwnerGraph()->getNodePinById(config.start_pin_id);
	if (m_startPin)
	{
		return false;
	}

	m_endPin= getOwnerGraph()->getNodePinById(config.end_pin_id);
	if (m_endPin)
	{
		return false;
	}

	return true;
}

void NodeLink::saveToConfig(NodeLinkConfig& config) const
{
	config.id = m_id;

	if (m_startPin)
	{
		config.start_pin_id= m_startPin->getId();
	}

	if (m_endPin)
	{
		config.end_pin_id = m_endPin->getId();
	}
}

void NodeLink::editorRender(const NodeEditorState& editorState)
{
	const int alpha = editorState.startedLinkPinId == -1 ? 255 : 50;

	m_startPin->editorRenderBeginLink(alpha);
	ImNodes::Link(m_id, m_startPin->getId(), m_endPin->getId());
	m_startPin->editorRenderEndLink();
}