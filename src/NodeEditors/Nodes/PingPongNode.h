#pragma once
#include "Node.h"

enum class ePingPongNodeType : int
{
	BUFFER,
	IMAGE
};

class PingPongNode : public Node
{
public:
	PingPongNode();
	PingPongNode(NodeGraphPtr parentGraph);

	virtual void editorRender(class NodeEditorState* editorState) override;

public:
	ePingPongNodeType m_eventNodeType= ePingPongNodeType::BUFFER;
	int size = 0;
};