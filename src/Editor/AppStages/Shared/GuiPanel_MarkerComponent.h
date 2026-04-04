#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "SinglecastDelegate.h"

class GuiPanel_MarkerComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_MarkerComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void render(float deltaSeconds) override;

	SinglecastDelegate<void(int arucoId)> OnMarkerSelected;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
	MarkerComponentPtr getMarkerComponent() const;

private:
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
};
