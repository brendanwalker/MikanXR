#include "MkGuiScopedRender.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

MkGuiScopedRender::MkGuiScopedRender(
	MkGuiContext& context)
	: MkGuiScopedContext(context)
{
}

MkGuiScopedRender::~MkGuiScopedRender()
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}