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

protected:
	virtual void editorRenderPushNodeStyle(NodeEditorState* editorState) const override;
	virtual std::string editorGetTitle() const override { return "Ping-Pong"; }

protected:
	ePingPongNodeType m_eventNodeType= ePingPongNodeType::BUFFER;
	int size = 0;
};