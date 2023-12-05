#pragma once
#include "Node.h"

class MousePosNode : public Node
{
public:
	MousePosNode();
	MousePosNode(NodeGraphPtr parentGraph);

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "MousePosition"; }
};