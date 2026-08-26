#include "MkGuiDockspace.h"

#include "imgui_internal.h"

namespace MkGui
{
ImGuiID beginDockspaceHost(const char* hostWindowId, const char* dockspaceId, bool& outNeedsDefaultLayout)
{
	const ImGuiViewport* viewport= ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	// NoBackground is what lets the 3d scene show through the central node.
	// NoBringToFrontOnFocus keeps the host behind the panels docked into it.
	const ImGuiWindowFlags hostFlags=
		ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::Begin(hostWindowId, nullptr, hostFlags);
	ImGui::PopStyleVar();

	const ImGuiID dockspace= ImGui::GetID(dockspaceId);

	outNeedsDefaultLayout= ImGui::DockBuilderGetNode(dockspace) == nullptr;
	if (outNeedsDefaultLayout)
	{
		ImGui::DockBuilderRemoveNode(dockspace);
		ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace, viewport->WorkSize);
	}

	ImGui::DockSpace(dockspace, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_PassthruCentralNode);

	return dockspace;
}

void endDockspaceHost() { ImGui::End(); }

ImGuiID dockBuilderSplit(ImGuiID nodeId, ImGuiDir direction, float sizeRatio, ImGuiID& inOutRemainingNodeId)
{
	return ImGui::DockBuilderSplitNode(nodeId, direction, sizeRatio, nullptr, &inOutRemainingNodeId);
}

void dockBuilderDockWindow(const char* windowName, ImGuiID nodeId) { ImGui::DockBuilderDockWindow(windowName, nodeId); }

void dockBuilderFinish(ImGuiID dockspaceId) { ImGui::DockBuilderFinish(dockspaceId); }

bool getDockspaceCentralRect(const char* dockspaceId, ImVec2& outPos, ImVec2& outSize)
{
	const ImGuiDockNode* centralNode= ImGui::DockBuilderGetCentralNode(ImGui::GetID(dockspaceId));
	if (centralNode == nullptr)
		return false;

	outPos= centralNode->Pos;
	outSize= centralNode->Size;
	return true;
}
} // namespace MkGui
