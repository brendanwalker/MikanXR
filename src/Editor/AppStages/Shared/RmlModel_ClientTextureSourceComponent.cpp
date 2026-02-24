#include "AppStage.h"
#include "RmlModel_ClientTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"

bool RmlModel_ClientTextureSourceComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<ClientTextureSourceComponent>(ownerAppStage->getRmlContext());
}

bool RmlModel_ClientTextureSourceComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	constructor.Bind("display_buffer_type", &m_displayBufferType);

	return true;
}