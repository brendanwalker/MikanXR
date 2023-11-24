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

	virtual void editorRender(class NodeEditorState* editorState) override;

public:
	eEventNodeType m_eventNodeType;
};