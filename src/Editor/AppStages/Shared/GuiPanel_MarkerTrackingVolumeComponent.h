#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"

class GuiPanel_MarkerTrackingVolumeComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_MarkerTrackingVolumeComponent(AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeComponent() const;

private:
	GuiDataSource_ComboBox m_originMarkerDataSource;
};
