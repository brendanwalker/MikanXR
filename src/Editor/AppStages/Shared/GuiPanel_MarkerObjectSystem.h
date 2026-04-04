#pragma once

#include "Shared/GuiPanel_MikanObjectSystem.h"
#include "MarkerObjectSystem.h"

class GuiPanel_MarkerObjectSystem : public GuiPanel_MikanObjectSystem
{
public:
	GuiPanel_MarkerObjectSystem() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void onConstruct() override;
	virtual void render(float deltaSeconds) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
};
