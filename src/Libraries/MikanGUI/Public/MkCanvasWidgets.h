#pragma once

#include "MkGuiExport.h"
#include "MkCanvasScopedPin.h"

#include "imgui.h"

// Node-canvas widget vocabulary shared by every graph editor: pin icons drawn
// straight into the draw list, keyed by an owned icon enum rather than any
// third-party style type.
namespace MkCanvas
{
// The canvas library reserves object id 0 as its invalid/background sentinel,
// while graph ids start at 0 (the first node of every existing graph file).
// Every id crossing the canvas boundary shifts by one, in both directions.
constexpr int k_canvasIdOffset= 1;
inline int toCanvasId(int graphId) { return graphId + k_canvasIdOffset; }
inline int fromCanvasId(int canvasId) { return canvasId - k_canvasIdOffset; }

enum class PinIcon : int
{
	Flow,
	Circle,
	Square,
	Grid,
	RoundSquare,
	Diamond,
};

// Draws a pin icon as an inline ImGui item of the given size, and anchors
// the pin's link endpoint to a fixed point on the icon's connection edge
// (right for outputs, left for inputs). Must be called between the pin's
// begin/end scope. Filled = connected; unfilled outlines read as open sockets.
MIKAN_GUI_FUNC(void)
drawPinIcon(const ImVec2& size, PinIcon icon, bool bFilled, MkCanvasPinDirection direction, const ImVec4& color,
			const ImVec4& innerColor= ImVec4(0.f, 0.f, 0.f, 0.f));
} // namespace MkCanvas
