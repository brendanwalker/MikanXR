#include "RmlModel_TextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"

// -- RmlModel_ClientTextureSourceComponent -----
bool RmlModel_ClientTextureSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<ClientTextureSourceComponent>(rmlContext);
}

// -- RmlModel_SpoutTextureSourceComponent -----
bool RmlModel_SpoutTextureSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<SpoutTextureSourceComponent>(rmlContext);
}