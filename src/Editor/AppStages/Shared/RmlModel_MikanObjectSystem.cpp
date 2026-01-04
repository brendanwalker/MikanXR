#include "MikanObjectSystem.h"
#include "RmlManager.h"
#include "RmlModel_MikanObjectSystem.h"
#include "Shared/RmlModel_EntityAccessor.h"

RmlModel_MikanObjectSystem::RmlModel_MikanObjectSystem()
	: m_objectSystem()
	, m_entityWeakAccessor(std::make_shared<RmlModel_EntityAccessor>())
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
	m_entityWeakAccessor->setEntityAccessor(objectSystem);
}

// IRmlModel
Rml::Context* RmlModel_MikanObjectSystem::getContext()
{
	return m_entityWeakAccessor->getContext();
}

Rml::DataModelHandle& RmlModel_MikanObjectSystem::getModelHandle()
{
	return m_entityWeakAccessor->getModelHandle();
}

void RmlModel_MikanObjectSystem::dispose()
{
	m_entityWeakAccessor->dispose();
	m_objectSystem.reset();
}

void RmlModel_MikanObjectSystem::update(float deltaSeconds)
{
	m_entityWeakAccessor->update(deltaSeconds);
}

void RmlModel_MikanObjectSystem::addModelUpdateCallback(std::function<void()> callback)
{
	m_entityWeakAccessor->addModelUpdateCallback(callback);
}