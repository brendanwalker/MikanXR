#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "Shared/GuiDataSource_StringList.h"
#include "VRTrackingVolumeComponent.h"
#include "MarkerObjectSystem.h"

class GuiPanel_VRTrackingVolumeComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_VRTrackingVolumeComponent(AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;
	virtual void onGui() override;

protected:
	VRTrackingVolumeComponentPtr getVRTrackingVolumeComponent() const;

private:
	GuiDataSource_StringList m_charucoMountDataSource;
	GuiDataSource_ComboBox m_originMarkerDataSource;
	GuiDataSource_ComboBox m_utilityMarkerDataSource;
};
