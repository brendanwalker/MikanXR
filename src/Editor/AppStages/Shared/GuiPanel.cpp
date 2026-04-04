#include "GuiPanel.h"

GuiPanel::~GuiPanel()
{
	dispose();
}

void GuiPanel::update(float deltaSeconds)
{
	for (auto& callback : m_updateCallbacks)
	{
		callback();
	}
	m_updateCallbacks.clear();
}

void GuiPanel::dispose()
{
	m_updateCallbacks.clear();
}

void GuiPanel::addUpdateCallback(std::function<void()> callback)
{
	m_updateCallbacks.push_back(callback);
}
