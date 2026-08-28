#include "MkCanvasScopedPin.h"
#include "MkCanvasWidgets.h"

#include "imgui_node_editor.h"

namespace ed= ax::NodeEditor;

MkCanvasScopedPin::MkCanvasScopedPin(int pinId, MkCanvasPinDirection direction)
{
	ed::BeginPin(MkCanvas::toCanvasId(pinId),
				 direction == MkCanvasPinDirection::Input ? ed::PinKind::Input : ed::PinKind::Output);
}

MkCanvasScopedPin::~MkCanvasScopedPin() { ed::EndPin(); }
