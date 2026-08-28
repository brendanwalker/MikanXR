#include "FlowPin.h"

FlowPin::FlowPin()
	: NodePin()
{
	m_bEditorShowPinName= false;
}

MkCanvas::PinIcon FlowPin::editorGetPinIcon() const { return MkCanvas::PinIcon::Flow; }

ImVec4 FlowPin::editorGetPinColor() const { return ImVec4(225.f / 255.f, 225.f / 255.f, 225.f / 255.f, 1.f); }

void FlowPin::editorRenderContextMenu(const NodeEditorState& editorState) {}

ImU32 FlowPin::editorGetLinkStyleColor() const { return IM_COL32(225, 225, 225, 255); }