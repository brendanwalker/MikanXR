#pragma once

#include "NodePin.h"

enum class eBlockPinType : int
{
	UNIFROM_BLOCK,
	BUFFER_BLOCK
};

class BlockPin : public NodePin
{
public:
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	eBlockPinType m_pinType= eBlockPinType::UNIFROM_BLOCK;
	int m_index= -1;
};