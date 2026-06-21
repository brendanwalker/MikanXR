#include "MkGuiScopedUpdate.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"

MkGuiScopedUpdate::MkGuiScopedUpdate(MkGuiContext& context)
	: MkGuiScopedContext(context)
{
	// Tell ImGui to prepare for a new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

MkGuiScopedUpdate::~MkGuiScopedUpdate()
{
	// Finalize the ImGui draw lists
	ImGui::Render();
}
