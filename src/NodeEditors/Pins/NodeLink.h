#pragma once

#include "NodeFwd.h"

class NodeLink
{
public:
	NodeLink();
	NodeLink(NodeGraphPtr m_ownerGraph);

	inline int getId() const { return m_id; }
	inline NodeGraphPtr getOwnerGraph() const { return m_ownerGraph; }
	inline NodePinPtr getStartPin() const { return m_pPin1; }
	inline NodePinPtr getEndPin() const { return m_pPin2; }

	virtual void editorRender(class NodeEditorState* editorState);

protected:
	int m_id;
	NodeGraphPtr m_ownerGraph;
	NodePinPtr m_pPin1;
	NodePinPtr m_pPin2;
};