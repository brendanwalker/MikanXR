#include "NodeLink.h"
#include "NodePin.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"

#include "imnodes.h"

NodeLink::NodeLink()
	: m_id(-1)
{
}

NodeLink::NodeLink(NodeGraphPtr ownerGraph)
	: m_id(ownerGraph->allocateId())
	, m_ownerGraph(ownerGraph)
{
}

void NodeLink::editorRender(const NodeEditorState& editorState)
{
	const int alpha = editorState.startedLinkPinId == -1 ? 255 : 50;

	m_startPin->editorRenderBeginLink(alpha);
	ImNodes::Link(m_id, m_startPin->getId(), m_endPin->getId());
	m_startPin->editorRenderEndLink();
}