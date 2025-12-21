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

void RmlModel_MikanObjectSystem::dispose()
{
	m_propertyInterface->dispose();
	m_objectSystem.reset();
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