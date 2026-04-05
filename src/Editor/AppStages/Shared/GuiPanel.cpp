#include "GuiPanel.h"

GuiPanel::GuiPanel(AppStage* ownerAppStage)
	: IGuiPanel()
	, m_ownerAppStage(ownerAppStage)
{
}

GuiPanel::~GuiPanel()
{
	dispose();
}

void GuiPanel::addDeferredGuiEvent(std::function<void()> callback)
{
	m_deferredGuiEvents.push_back(callback);
}

void GuiPanel::processDeferredGuiEvents()
{
	for (auto& callback : m_deferredGuiEvents)
	{
		callback();
	}
	m_deferredGuiEvents.clear();
}

void GuiPanel::dispose()
{
	m_deferredGuiEvents.clear();
}

