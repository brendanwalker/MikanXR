#include "AppStage.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"
#include "LocText.h"

#include "imgui.h"

bool GuiPanel_ClientTextureSourceComponent::init()
{
	m_displayBufferDataSource.setEntries(
		{locText("componentPanel.displayBufferColor"), locText("componentPanel.displayBufferDepth")});
	return initTypedPropertyInterface<ClientTextureSourceComponent>();
}

void GuiPanel_ClientTextureSourceComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	// Display buffer type selector
	int selectedIndex= (int)m_displayBufferType;
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "displayBufferType", locText("componentPanel.displayBuffer"),
									&m_displayBufferDataSource, selectedIndex))
	{
		m_displayBufferType= (eTextureSourceDisplayBufferType)selectedIndex;
	}
}
