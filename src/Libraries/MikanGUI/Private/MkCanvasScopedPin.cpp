#include "MkCanvasScopedPin.h"

#include "imgui_node_editor.h"

namespace ed= ax::NodeEditor;

MkCanvasScopedPin::MkCanvasScopedPin(int pinId, MkCanvasPinDirection direction)
{
	ed::BeginPin(pinId, direction == MkCanvasPinDirection::Input ? ed::PinKind::Input : ed::PinKind::Output);
}

MkCanvasScopedPin::~MkCanvasScopedPin() { ed::EndPin(); }
