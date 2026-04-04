#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "VRTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"
#include "TrackingMountObjectSystem.h"

class GuiPanel_VRTrackingVolumeComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_VRTrackingVolumeComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void render(float deltaSeconds) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	VRTrackingVolumeComponentPtr getVRTrackingVolumeComponent() const;

private:
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};
