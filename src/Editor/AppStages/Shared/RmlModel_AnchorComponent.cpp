#include "AppStage.h"
#include "Shared/RmlModel_AnchorComponent.h"

RmlModel_AnchorComponent::RmlModel_AnchorComponent()
	: RmlModel_MikanComponent()
{}

bool RmlModel_AnchorComponent::init(class AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<AnchorComponent>(ownerAppStage->getRmlContext());
}