#pragma once

#include "Shared/GuiPanel.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include "TrackingMountObjectSystem.h"

class GuiPanel_ProjectTracking : public GuiPanel
{
public:
	GuiPanel_ProjectTracking() = default;

	bool init(class ProjectGuiPanelContext* context);
	virtual void render(float deltaSeconds) override;
	virtual void dispose() override;

private:
	MarkerTrackingVolumeComponentPtr getSelectedMarkerTrackingVolume() const;
	VRTrackingVolumeComponentPtr getSelectedVRTrackingVolume() const;
	TrackingMountComponentPtr getSelectedTrackingMount() const;
	TrackingMountObjectSystemPtr getTrackingMountSystem() const;

	void setSelectedTrackingVolumeId(MikanTrackingVolumeID volumeId);
	void setSelectedTrackingMountId(MikanTrackingMountID mountId);

	class ProjectGuiPanelContext* m_context = nullptr;
	TrackingMountObjectSystemWeakPtr m_trackingMountSystem;
	ProjectManagerWeakPtr m_projectManager;

	int m_selectedTrackingVolumeId = INVALID_MIKAN_ID;
	int m_selectedTrackingMountId = INVALID_MIKAN_ID;
	bool m_isVRTrackingVolume = false;
};
