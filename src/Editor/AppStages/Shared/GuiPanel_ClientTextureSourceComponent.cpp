#include "AppStage.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"

#include "imgui.h"

bool GuiPanel_ClientTextureSourceComponent::init()
{
	m_displayBufferDataSource.setEntries({ "Color", "Depth" });
	return initTypedPropertyInterface<ClientTextureSourceComponent>();
}

void GuiPanel_ClientTextureSourceComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	ImGui::Separator();

	// Display buffer type selector
	int selectedIndex = (int)m_displayBufferType;
	if (MkGui::drawComboBoxProperty(
		m_defaultGuiStyle, "displayBufferType", "Display Buffer",
		&m_displayBufferDataSource, selectedIndex))
	{
		m_displayBufferType = (eTextureSourceDisplayBufferType)selectedIndex;
	}
}
