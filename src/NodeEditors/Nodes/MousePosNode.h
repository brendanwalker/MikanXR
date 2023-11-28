#pragma once
#include "Node.h"

class MousePosNode : public Node
{
public:
	MousePosNode();
	MousePosNode(NodeGraphPtr parentGraph);

	virtual void editorRender(class NodeEditorState* editorState) override;
};