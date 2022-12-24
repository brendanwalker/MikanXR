#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>
#include <vector>

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;
typedef std::vector<VRDeviceViewPtr> VRDeviceList;

//-- definitions -----
class AppStage_SpatialAnchors : public AppStage
{
public:
	AppStage_SpatialAnchors(class App* app);
	virtual ~AppStage_SpatialAnchors();

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;

	//virtual void renderUI() override;

	static const char* APP_STAGE_NAME;

protected:
	VRDeviceViewPtr getSelectedAnchorVRTracker() const;

	struct SpatialAnchorSetupDataModel* m_dataModel = nullptr;

	VRDeviceList m_vrTrackers;
	class GlScene *m_scene;
	class GlCamera *m_camera;
	class ProfileConfig *m_profile;
};
