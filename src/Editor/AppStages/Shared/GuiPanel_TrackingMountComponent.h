#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "TrackingMountComponent.h"
#include "VRObjectSystem.h"

class GuiPanel_TrackingMountComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_TrackingMountComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onGui() override;

protected:
	VRObjectSystemPtr getVRObjectSystem() const;
	VRDeviceComponentPtr getVRDeviceComponent() const;
	TrackingMountComponentPtr getTrackingMountComponent() const;

	void setDevicePath(const std::string& devicePath);

private:
	VRObjectSystemWeakPtr m_vrObjectSystem;
	std::vector<std::string> m_vrDevicePaths;
	std::vector<std::string> m_socketNames;
};

using GuiPanel_TrackingMountComponentPtr = std::shared_ptr<GuiPanel_TrackingMountComponent>;
