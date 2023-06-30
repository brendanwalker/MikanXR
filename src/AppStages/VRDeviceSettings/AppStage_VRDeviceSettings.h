#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>
#include <vector>

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;
typedef std::vector<VRDeviceViewPtr> VRDeviceList;

//-- definitions -----
class AppStage_VRDeviceSettings : public AppStage
{
public:
	AppStage_VRDeviceSettings(class App* app);
	virtual ~AppStage_VRDeviceSettings();

	virtual void enter() override;
	virtual void exit() override;

	virtual void onRmlClickEvent(const std::string& value) override;

	static const char* APP_STAGE_NAME;

protected:
	void rebuildVRTrackerList();

	// VR Device Setting Model UI Events
	void onUpdateCameraVRDevicePath(const std::string& devicePath);
	void onUpdateMatVRDevicePath(const std::string& devicePath);
	void onUpdateOriginVRDevicePath(const std::string& devicePath);
	void onUpdateOriginVerticalAlignFlag(bool bFlag);

	class RmlModel_VRDeviceSettings* m_vrDeviceSettingsModel = nullptr;
	Rml::ElementDocument* m_vrDeviceSettingsView = nullptr;

	VRDeviceList m_vrTrackers;
};