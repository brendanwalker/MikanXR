#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "VRTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"

class GuiPanel_VRTrackingVolumeComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_VRTrackingVolumeComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual void onGui() override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	VRTrackingVolumeComponentPtr getVRTrackingVolumeComponent() const;

private:
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};
