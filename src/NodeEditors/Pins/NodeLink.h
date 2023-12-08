#pragma once

#include "NodeFwd.h"

class NodeLink
{
public:
	NodeLink();
	NodeLink(NodeGraphPtr m_ownerGraph);

	inline int getId() const { return m_id; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	inline NodePinPtr getStartPin() const { return m_startPin; }
	inline NodePinPtr getEndPin() const { return m_endPin; }

	inline void setStartPin(NodePinPtr pin) { m_startPin= pin; }
	inline void setEndPin(NodePinPtr pin) { m_endPin= pin; }

	virtual void editorRender(const NodeEditorState& editorState);

protected:
	int m_id;
	NodeGraphPtr m_ownerGraph;
	NodePinPtr m_startPin;
	NodePinPtr m_endPin;
};