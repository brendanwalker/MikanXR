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

void NodeLink::editorRender(NodeEditorState* editorState)
{
	const int alpha = editorState->startedLinkPinId == -1 ? 255 : 50;

	m_pPin1->editorRenderBeginLink(alpha);
	ImNodes::Link(m_id, m_pPin1->getId(), m_pPin2->getId());
	m_pPin1->editorRenderEndLink();
}