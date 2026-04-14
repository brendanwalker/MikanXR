#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_StringList.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "SinglecastDelegate.h"

class GuiPanel_MarkerComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_MarkerComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onConstruct() override;

	SinglecastDelegate<void(int arucoId)> OnMarkerSelected;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
	MarkerComponentPtr getMarkerComponent() const;

private:
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	GuiDataSource_StringList m_arucoIdDataSource;
};
