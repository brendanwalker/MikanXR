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

	void setSelectedCameraVRTrackerIndex(int index);
	void setSelectedMatVRTrackerIndex(int index);

	int getVRTrackerCount() const;

	VRDeviceViewPtr getSelectedCameraVRTracker() const;
	VRDeviceViewPtr getSelectedMatVRTracker() const;

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;

	virtual void renderUI() override;

	static const char* APP_STAGE_NAME;

protected:
	void rebuildVRTrackerList();

	VRDeviceList m_vrTrackers;

	int m_selectedCameraVRTrackerIndex;
	int m_selectedMatVRTrackerIndex;
};