#include "AppStage.h"
#include "Shared/GuiPanel_AnchorComponent.h"

bool GuiPanel_AnchorComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<AnchorComponent>(ownerAppStage);
}
