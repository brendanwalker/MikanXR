#pragma once
#include "Node.h"

class TimeNode : public Node
{
public:
	TimeNode();
	TimeNode(NodeGraphPtr parentGraph);

protected:
	virtual void editorRenderPushNodeStyle(NodeEditorState* editorState) const override;
	virtual std::string editorGetTitle() const override { return "Time"; }
};