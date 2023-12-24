#pragma once

#include "NodeFwd.h"
#include "CommonConfig.h"

class NodeLinkConfig : public CommonConfig
{
public:
	NodeLinkConfig() : CommonConfig() {}
	NodeLinkConfig(const std::string& nodeName) : CommonConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_node_id id= -1;
	t_node_pin_id start_pin_id= -1;
	t_node_pin_id end_pin_id= -1;
};

class NodeLink
{
public:
	NodeLink()= default;

	inline std::string getClassName() const { return "NodeLink"; }

	virtual bool loadFromConfig(NodeLinkConfigConstPtr config);
	virtual void saveToConfig(NodeLinkConfigPtr config) const;

	inline void setId(t_node_link_id id) { m_id = id; }
	inline t_node_link_id getId() const { return m_id; }

	inline void setOwnerGraph(NodeGraphPtr ownerGraph) { m_ownerGraph= ownerGraph; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }

	inline void setStartPin(NodePinPtr pin) { m_startPin = pin; }
	inline NodePinPtr getStartPin() const { return m_startPin; }

	inline void setEndPin(NodePinPtr pin) { m_endPin = pin; }
	inline NodePinPtr getEndPin() const { return m_endPin; }

	virtual void editorRender(const NodeEditorState& editorState);

protected:
	t_node_link_id m_id= -1;
	NodeGraphPtr m_ownerGraph;
	NodePinPtr m_startPin;
	NodePinPtr m_endPin;
};