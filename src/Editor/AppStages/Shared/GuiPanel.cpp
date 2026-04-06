#include "GuiPanel.h"
#include "AppStage.h"
#include "IEditorWindow.h"

GuiPanel::GuiPanel(AppStage* ownerAppStage)
	: IGuiPanel()
	, m_ownerAppStage(ownerAppStage)
{
}

GuiPanel::~GuiPanel()
{
	dispose();
}

MkGuiStyleManager* GuiPanel::getGuiStyleManager() const
{
	AppStage* ownerAppStage = getOwnerAppStage();
	if (ownerAppStage)
	{
		return ownerAppStage->getOwnerWindow()->getMkGuiStyleManager();
	}
	return nullptr;
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

