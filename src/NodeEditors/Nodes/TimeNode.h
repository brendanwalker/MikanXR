#pragma once
#include "Node.h"

class TimeNode : public Node
{
public:
	TimeNode();
	TimeNode(NodeGraphPtr parentGraph);

	virtual void editorRender(class NodeEditorState* editorState) override;
};