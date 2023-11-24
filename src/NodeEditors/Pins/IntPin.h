#pragma once

#include "NodePin.h"

class IntPin : public NodePin
{
public:
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) override;
};