#pragma once

#include "MkGuiExport.h"

#include "imgui.h"

// Dockspace hosting. The DockBuilder half of this is ImGui internal API, which
// stays behind this wrapper so the editor never includes imgui_internal.h.
namespace MkGui
{
// Opens a full-viewport, chrome-free host window carrying a menu bar and a
// dockspace whose central node is left transparent, so whatever the app drew
// into the back buffer (the 3d scene) shows through it.
//
// Returns the dockspace id. outNeedsDefaultLayout is true only on a run with no
// saved layout, which is when the caller should build one and call
// dockBuilderFinish. Always pair with endDockspaceHost.
MIKAN_GUI_FUNC(ImGuiID)
beginDockspaceHost(const char* hostWindowId, const char* dockspaceId, bool& outNeedsDefaultLayout);
MIKAN_GUI_FUNC(void) endDockspaceHost();

// Splits a node, returning the new child on the given side. The remainder of
// the original node is written back to inOutRemainingNodeId.
MIKAN_GUI_FUNC(ImGuiID) dockBuilderSplit(ImGuiID nodeId, ImGuiDir direction, float sizeRatio,
										 ImGuiID& inOutRemainingNodeId);
MIKAN_GUI_FUNC(void) dockBuilderDockWindow(const char* windowName, ImGuiID nodeId);
MIKAN_GUI_FUNC(void) dockBuilderFinish(ImGuiID dockspaceId);

// Screen-space rect of the central (empty) node, which is the area left for the
// 3d scene. Returns false when the dockspace has no central node yet.
//
// Takes the same string id passed to beginDockspaceHost and must be called while
// that host window is still current (from the stage's gui pass), because the id
// resolves against the current window's id stack.
MIKAN_GUI_FUNC(bool) getDockspaceCentralRect(const char* dockspaceId, ImVec2& outPos, ImVec2& outSize);
} // namespace MkGui
