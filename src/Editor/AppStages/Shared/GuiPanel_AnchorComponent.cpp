#include "AppStage.h"
#include "Shared/GuiPanel_AnchorComponent.h"

bool GuiPanel_AnchorComponent::init()
{
	return initTypedPropertyInterface<AnchorComponent>();
}
