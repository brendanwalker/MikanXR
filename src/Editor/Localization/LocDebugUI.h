#pragma once

// The two diagnostics for tracking a bad UI string back to its key, shared by
// every editor window that has a View menu.
//
// Show-keys mode replaces every fetched string with its own key, so the key
// appears in the widget the bad string was in. It is global (one
// LocalizationManager serves every window) and transient.
//
// The ID Stack Tool is ImGui's own hover inspector. Because locLabel puts the
// key after "##", hovering an interactive widget reports its key without
// leaving the normal view. It only sees widgets that own an ID, so plain text
// needs show-keys mode. It is per window, since it reports the hovered item of
// its own ImGui context.
namespace LocDebugUI
{
// The show-keys hotkey. Call once per ImGui frame from a window that takes
// focus. No modifier, so it needs no guard against text fields.
void handleShortcuts();
// Both View menu items. Call inside an open menu.
void drawViewMenuItems(bool& bInOutShowIdStackTool);
// The ID Stack Tool window itself. Call in the frame body, outside the menu bar.
void drawWindows(bool& bInOutShowIdStackTool);
} // namespace LocDebugUI
