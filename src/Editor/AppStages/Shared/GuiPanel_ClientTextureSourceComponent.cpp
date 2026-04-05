#include "AppStage.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"

#include "imgui.h"

bool GuiPanel_ClientTextureSourceComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<ClientTextureSourceComponent>(ownerAppStage);
}

void GuiPanel_ClientTextureSourceComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	ImGui::Separator();

	// Display buffer type selector (Color = 0, Depth = 1)
	const char* bufferTypeNames[] = { "Color", "Depth" };
	if (ImGui::Combo("Display Buffer", (int*)&m_displayBufferType, bufferTypeNames, 2))
	{
		// m_displayBufferType updated by ImGui directly
	}
}
