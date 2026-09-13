#include "LocDebugUI.h"
#include "LocText.h"
#include "LocalizationManager.h"

#include "imgui.h"

namespace LocDebugUI
{
void handleShortcuts()
{
	LocalizationManager* locManager= LocalizationManager::getInstance();
	if (locManager == nullptr)
		return;

	if (ImGui::IsKeyPressed(ImGuiKey_F9, false))
	{
		locManager->setShowKeys(!locManager->getShowKeys());
	}
}

void drawViewMenuItems(bool& bInOutShowIdStackTool)
{
	LocalizationManager* locManager= LocalizationManager::getInstance();
	if (locManager != nullptr)
	{
		bool bShowKeys= locManager->getShowKeys();
		if (ImGui::MenuItem(locLabel("mainWindow.showStringKeys"), "F9", &bShowKeys))
		{
			locManager->setShowKeys(bShowKeys);
		}
	}

	ImGui::MenuItem(locLabel("mainWindow.idStackTool"), nullptr, &bInOutShowIdStackTool);
}

void drawWindows(bool& bInOutShowIdStackTool)
{
	if (bInOutShowIdStackTool)
	{
		ImGui::ShowIDStackToolWindow(&bInOutShowIdStackTool);
	}
}
} // namespace LocDebugUI
