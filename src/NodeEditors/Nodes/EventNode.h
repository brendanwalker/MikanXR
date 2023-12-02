#pragma once
#include "Node.h"

enum class eEventNodeType
{
	INIT,
	FRAME
};

class EventNode : public Node
{
public:
	EventNode();
	EventNode(NodeGraphPtr parentGraph);

protected:
	virtual void editorRenderPushNodeStyle(NodeEditorState* editorState) const override;
	virtual std::string editorGetTitle() const override;

protected:
	eEventNodeType m_eventNodeType;
};