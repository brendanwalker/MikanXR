#include "MikanObjectSystem.h"
#include "RmlManager.h"
#include "RmlModel_MikanObjectSystem.h"
#include "Shared/RmlModel_EntityAccessor.h"

RmlModel_MikanObjectSystem::RmlModel_MikanObjectSystem()
	: m_objectSystem()
	, m_entityAccessor(std::make_shared<RmlModel_EntityAccessor>())
{
}

bool RmlModel_MikanObjectSystem::onConstruct(
	Rml::DataModelConstructor& constructor)
{
	RmlManager::getInstance()->bindEnumDefinitionsToDataModel(constructor);

	return true;
}

MikanObjectSystemPtr RmlModel_MikanObjectSystem::getObjectSystem() const
{
	return m_objectSystem.lock();
}

void RmlModel_MikanObjectSystem::setObjectSystem(MikanObjectSystemPtr objectSystem)
{
	m_objectSystem = objectSystem;
	m_entityAccessor->setEntityAccessor(objectSystem);
}

// IRmlModel
Rml::Context* RmlModel_MikanObjectSystem::getContext()
{
	return m_entityAccessor->getContext();
}

Rml::DataModelHandle& RmlModel_MikanObjectSystem::getModelHandle()
{
	return m_entityAccessor->getModelHandle();
}

void RmlModel_MikanObjectSystem::dispose()
{
	m_entityAccessor->dispose();
	m_objectSystem.reset();
}

void RmlModel_MikanObjectSystem::update(float deltaSeconds)
{
	m_entityAccessor->update(deltaSeconds);
}

void RmlModel_MikanObjectSystem::addModelUpdateCallback(std::function<void()> callback)
{
	m_entityAccessor->addModelUpdateCallback(callback);
}