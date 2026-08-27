#include "MkGuiScopedContext.h"
#include "MkGuiContext.h"

#include "imgui.h"

MkGuiScopedContext::MkGuiScopedContext(MkGuiContext& context)
{
	m_prevImGuiContext= ImGui::GetCurrentContext();
	context.makeCurrent();
}

MkGuiScopedContext::~MkGuiScopedContext() { ImGui::SetCurrentContext(m_prevImGuiContext); }
