#include "NodeLink.h"
#include "NodePin.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"

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
bool NodeLink::loadFromConfig(NodeLinkConfigConstPtr config)
{
	bool success= false;
	m_id = config->id;
	
	m_startPin= getOwnerGraph()->getNodePinById(config->start_pin_id);
	if (!m_startPin)
	{
		MIKAN_LOG_ERROR("NodeLink::loadFromConfig") << "Invalid start pin id: " << config->start_pin_id;
		success= false;
	}

	m_endPin= getOwnerGraph()->getNodePinById(config->end_pin_id);
	if (!m_endPin)
	{
		MIKAN_LOG_ERROR("NodeLink::loadFromConfig") << "Invalid end pin id: " << config->end_pin_id;
		success= false;
	}

	return success;
}

void NodeLink::saveToConfig(NodeLinkConfigPtr config) const
{
	config->id = m_id;
	config->start_pin_id= m_startPin ? m_startPin->getId() : -1;
	config->end_pin_id= m_endPin ? m_endPin->getId() : -1;
}

NodePinPtr NodeLink::getConnectedPin(NodePinPtr pin) const
{
	if (pin == m_startPin)
		return m_endPin;
	else if (pin == m_endPin)
		return m_startPin;
	else
		NodePinPtr();
}

void NodeLink::editorRender(const NodeEditorState& editorState)
{
	const int alpha = editorState.startedLinkPinId == -1 ? 255 : 50;

	m_startPin->editorRenderBeginLink(alpha);
	ImNodes::Link(m_id, m_startPin->getId(), m_endPin->getId());
	m_startPin->editorRenderEndLink();
}