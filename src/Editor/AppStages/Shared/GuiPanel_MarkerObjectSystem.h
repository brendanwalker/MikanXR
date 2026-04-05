#pragma once

#include "Shared/GuiPanel_MikanObjectSystem.h"
#include "MarkerObjectSystem.h"

class GuiPanel_MarkerObjectSystem : public GuiPanel_MikanObjectSystem
{
public:
	GuiPanel_MarkerObjectSystem(class AppStage* ownerAppStage) 
		: GuiPanel_MikanObjectSystem(ownerAppStage) {}

	virtual bool init() override;
	virtual void onConstruct() override;
	virtual void onGui() override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
};
