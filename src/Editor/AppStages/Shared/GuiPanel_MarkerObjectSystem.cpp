#include "AppStage.h"
#include "MarkerObjectSystem.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"

bool GuiPanel_MarkerObjectSystem::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<MarkerObjectSystem>(ownerAppStage);
}

void GuiPanel_MarkerObjectSystem::onConstruct()
{
}

void GuiPanel_MarkerObjectSystem::render(float deltaSeconds)
{
	// Auto-render all system properties (aruco/charuco dictionary, rows, cols, etc.)
	GuiPanel_MikanObjectSystem::render(deltaSeconds);
}

MarkerObjectSystemPtr GuiPanel_MarkerObjectSystem::getMarkerObjectSystem() const
{
	MikanObjectSystemPtr objectSystem = m_objectSystem.lock();
	if (objectSystem)
	{
		return std::static_pointer_cast<MarkerObjectSystem>(objectSystem);
	}
	return nullptr;
}

MarkerObjectSystemDefinitionPtr GuiPanel_MarkerObjectSystem::getMarkerObjectSystemDefinition() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getTypedDefinition();
	}
	return nullptr;
}
