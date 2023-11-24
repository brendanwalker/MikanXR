#pragma once

#include "NodePin.h"

class FloatPin : public NodePin
{
public:
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) override;
};