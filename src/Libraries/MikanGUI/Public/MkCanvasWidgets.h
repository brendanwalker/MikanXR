#pragma once

#include "MkGuiExport.h"

#include "imgui.h"

// Node-canvas widget vocabulary shared by every graph editor: pin icons drawn
// straight into the draw list, keyed by an owned icon enum rather than any
// third-party style type.
namespace MkCanvas
{
enum class PinIcon : int
{
	Flow,
	Circle,
	Square,
	Grid,
	RoundSquare,
	Diamond,
};

// Draws a pin icon as an inline ImGui item of the given size.
// Filled = connected; unfilled outlines read as open sockets.
MIKAN_GUI_FUNC(void)
drawPinIcon(const ImVec2& size, PinIcon icon, bool bFilled, const ImVec4& color,
			const ImVec4& innerColor= ImVec4(0.f, 0.f, 0.f, 0.f));
} // namespace MkCanvas
