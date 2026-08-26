#include "BoolPin.h"
#include "MkGuiDrawUtils.h"

const ImU32 BoolPin::editorValuePinColor(float alpha) const
{
	return ImGui::GetColorU32(MkGui::getBooleanColor(alpha));
}