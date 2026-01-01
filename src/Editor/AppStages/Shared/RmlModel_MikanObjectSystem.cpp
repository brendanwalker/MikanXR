#include "MikanObjectSystem.h"
#include "RmlManager.h"
#include "RmlModel_MikanObjectSystem.h"
#include "Shared/RmlModel_PropertyInterface.h"

RmlModel_MikanObjectSystem::RmlModel_MikanObjectSystem()
	: m_objectSystem()
	, m_propertyInterface(std::make_shared<RmlModel_PropertyInterface>())
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
	m_propertyInterface->setPropertyInterface(objectSystem, objectSystem->getObjectSystemConfig());
	m_propertyInterface->setFunctionInterface(objectSystem);
}

// IRmlModel
Rml::Context* RmlModel_MikanObjectSystem::getContext()
{
	return m_propertyInterface->getContext();
}

Rml::DataModelHandle& RmlModel_MikanObjectSystem::getModelHandle()
{
	return m_propertyInterface->getModelHandle();
}

void RmlModel_MikanObjectSystem::dispose()
{
	m_propertyInterface->dispose();
	m_objectSystem.reset();
}

void RmlModel_MikanObjectSystem::update()
{
	m_propertyInterface->update();
}

void RmlModel_MikanObjectSystem::addModelUpdateCallback(std::function<void()> callback)
{
	m_propertyInterface->addModelUpdateCallback(callback);
}