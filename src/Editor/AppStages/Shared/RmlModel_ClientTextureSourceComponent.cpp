#include "RmlModel_ClientTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"

bool RmlModel_ClientTextureSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<ClientTextureSourceComponent>(rmlContext);
}
