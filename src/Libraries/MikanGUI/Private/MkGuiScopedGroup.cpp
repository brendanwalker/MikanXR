#include "MkGuiScopedGroup.h"

#include "imgui.h"

MkGuiScopedGroup::MkGuiScopedGroup()
{
	ImGui::BeginGroup();
}

MkGuiScopedGroup::~MkGuiScopedGroup()
{
	ImGui::EndGroup();
}
