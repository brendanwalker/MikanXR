#include "MikanObjectSystem.h"
#include "RmlModel_MikanObjectSystem.h"
#include "Shared/RmlModel_PropertyInterface.h"

RmlModel_MikanObjectSystem::RmlModel_MikanObjectSystem()
	: m_objectSystem()
	, m_propertyInterface(std::make_shared<RmlModel_PropertyInterface>())
{

}

bool RmlModel_MikanObjectSystem::init(
	Rml::Context* rmlContext,
	MikanObjectSystemPtr objectSystem)
{
	if (m_propertyInterface->init<MikanObjectSystem>(rmlContext, "MikanObjectSystem"))
	{
		m_objectSystem = objectSystem;
		m_propertyInterface->setPropertyInterface(objectSystem, objectSystem->getObjectSystemConfig());
		m_propertyInterface->setFunctionInterface(objectSystem);

		return true;
	}

	return false;
}

void RmlModel_MikanObjectSystem::dispose()
{
	m_propertyInterface->dispose();
	m_objectSystem.reset();
}
