#pragma once

//-- includes -----
#include "AppStage.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "RendererFwd.h"

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

	static const char* APP_STAGE_NAME;

protected:
	VRDeviceViewPtr getSelectedAnchorVRTracker() const;

	struct SpatialAnchorSetupDataModel* m_dataModel = nullptr;

	VRDeviceList m_vrTrackers;
	GlScenePtr m_scene;
	GlCameraPtr m_camera;

	ProfileConfigPtr m_profile;
	AnchorObjectSystemConfigPtr m_anchorSystemConfig;
	AnchorObjectSystemPtr m_anchorSystem;
};
