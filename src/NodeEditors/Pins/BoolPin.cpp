#include "BoolPin.h"
#include "NodeEditorUI.h"

const ImU32 BoolPin::editorValuePinColor(float alpha) const
{
	return ImGui::GetColorU32(NodeEditorUI::getBooleanColor(alpha));
}