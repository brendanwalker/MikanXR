#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_StringList.h"
#include "TrackingMountComponent.h"
#include "VRObjectSystem.h"

class GuiPanel_TrackingMountComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_TrackingMountComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	VRObjectSystemPtr getVRObjectSystem() const;
	VRDeviceComponentPtr getVRDeviceComponent() const;
	TrackingMountComponentPtr getTrackingMountComponent() const;

	void setDevicePath(const std::string& devicePath);

private:
	VRObjectSystemWeakPtr m_vrObjectSystem;
	GuiDataSource_StringList m_devicePathDataSource;
	GuiDataSource_StringList m_socketNameDataSource;
};

using GuiPanel_TrackingMountComponentPtr = std::shared_ptr<GuiPanel_TrackingMountComponent>;
