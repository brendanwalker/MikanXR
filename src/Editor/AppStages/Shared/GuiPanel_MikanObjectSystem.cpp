#include "GuiPanel_MikanObjectSystem.h"
#include "MikanObjectSystem.h"

GuiPanel_MikanObjectSystem::GuiPanel_MikanObjectSystem()
	: m_objectSystem()
	, m_entityAccessor(std::make_shared<GuiPanel_EntityAccessor>())
{
}

MikanObjectSystemPtr GuiPanel_MikanObjectSystem::getObjectSystem() const
{
	return m_objectSystem.lock();
}

void GuiPanel_MikanObjectSystem::setObjectSystem(MikanObjectSystemPtr objectSystem)
{
	m_objectSystem = objectSystem;
	m_entityAccessor->setEntityAccessor(objectSystem);
}

void GuiPanel_MikanObjectSystem::render(float deltaSeconds)
{
	m_entityAccessor->render(deltaSeconds);
}

void GuiPanel_MikanObjectSystem::update(float deltaSeconds)
{
	m_entityAccessor->update(deltaSeconds);
}

void GuiPanel_MikanObjectSystem::dispose()
{
	m_entityAccessor->dispose();
	m_objectSystem.reset();
}

void GuiPanel_MikanObjectSystem::addUpdateCallback(std::function<void()> callback)
{
	m_entityAccessor->addUpdateCallback(callback);
}
