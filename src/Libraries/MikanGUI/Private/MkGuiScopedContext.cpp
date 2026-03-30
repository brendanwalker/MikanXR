#include "MkGuiScopedContext.h"
#include "MkGuiContext.h"

#include "imgui.h"
#include "imnodes.h"

MkGuiScopedContext::MkGuiScopedContext(MkGuiContext& context)
{
	m_prevImGuiContext= ImGui::GetCurrentContext();
	m_prevImNodesContext= ImNodes::GetCurrentContext();
	context.makeCurrent();
}

MkGuiScopedContext::~MkGuiScopedContext()
{
	ImGui::SetCurrentContext(m_prevImGuiContext);
	ImNodes::SetCurrentContext(m_prevImNodesContext);
}
