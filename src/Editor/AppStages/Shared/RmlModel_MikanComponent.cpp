#include "MikanComponent.h"
#include "RmlModel_MikanComponent.h"
#include "Shared/RmlModel_PropertyInterface.h"

RmlModel_MikanComponent::RmlModel_MikanComponent()
	: m_component()
	, m_propertyInterface(std::make_shared<RmlModel_PropertyInterface>())
{

}

bool RmlModel_MikanComponent::init(Rml::Context* rmlContext)
{
	if (!m_propertyInterface->init<MikanComponent>(rmlContext, "MikanComponent"))
	{
		return false;
	}

	return true;
}

void RmlModel_MikanComponent::setComponent(MikanComponentPtr component)
{
	MikanComponentPtr oldComponent = m_component.lock();

	if (component != oldComponent)
	{
		m_component = component;
		m_propertyInterface->setPropertyInterface(component, component->getDefinition());
		m_propertyInterface->setFunctionInterface(component);
	}
}

