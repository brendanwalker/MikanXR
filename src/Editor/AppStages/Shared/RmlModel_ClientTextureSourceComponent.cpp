#include "AppStage.h"
#include "RmlModel_ClientTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"

bool RmlModel_ClientTextureSourceComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<ClientTextureSourceComponent>(ownerAppStage->getRmlContext());
}
