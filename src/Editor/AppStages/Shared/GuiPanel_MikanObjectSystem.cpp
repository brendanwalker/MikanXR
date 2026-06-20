#include "GuiPanel_MikanObjectSystem.h"
#include "MikanObjectSystem.h"

GuiPanel_MikanObjectSystem::GuiPanel_MikanObjectSystem(AppStage* ownerAppStage)
	: m_objectSystem()
	, m_entityAccessor(std::make_shared<GuiPanel_EntityAccessor>(ownerAppStage))
{
}

MikanObjectSystemPtr GuiPanel_MikanObjectSystem::getObjectSystem() const
{
	return m_objectSystem.lock();
}

void GuiPanel_MikanObjectSystem::setObjectSystem(MikanObjectSystemPtr objectSystem)
{
	m_objectSystem= objectSystem;
	m_entityAccessor->setEntityAccessor(objectSystem);
}

void GuiPanel_MikanObjectSystem::onGui()
{
	m_entityAccessor->onGui();
}

void GuiPanel_MikanObjectSystem::addDeferredGuiEvent(std::function<void()> callback)
{
	m_entityAccessor->addDeferredGuiEvent(callback);
}

void GuiPanel_MikanObjectSystem::processDeferredGuiEvents()
{
	m_entityAccessor->processDeferredGuiEvents();
}

void GuiPanel_MikanObjectSystem::dispose()
{
	m_entityAccessor->dispose();
	m_objectSystem.reset();
}