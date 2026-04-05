#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"

class GuiPanel_MarkerTrackingVolumeComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_MarkerTrackingVolumeComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void onGui() override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	TrackingMountObjectSystemPtr getTrackingMountObjectSystem() const;
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeComponent() const;

private:
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};
