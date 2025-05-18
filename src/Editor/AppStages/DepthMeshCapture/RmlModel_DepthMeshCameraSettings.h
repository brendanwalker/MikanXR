#pragma once

#include "ObjectSystemConfigFwd.h"
#include "SinglecastDelegate.h"
#include "Shared/RmlModel.h"
#include "VideoDisplayConstants.h"
#include "Constants_DepthMeshCapture.h"

class ProjectConfig;

class VideoSourceView;
typedef std::shared_ptr<const VideoSourceView> VideoSourceViewConstPtr;

class RmlModel_DepthMeshCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		VideoSourceViewConstPtr videoSourceView,
		ProjectConfigConstPtr profileConfig);
	virtual void dispose() override;

	eDepthMeshCaptureMenuState getMenuState() const;
	void setMenuState(eDepthMeshCaptureMenuState newState);

	eDepthMeshCaptureViewpointMode getViewpointMode() const;
	void setViewpointMode(eDepthMeshCaptureViewpointMode newMode);

	SinglecastDelegate<void(eDepthMeshCaptureViewpointMode)> OnViewpointModeChanged;

private:
	// Model Variables
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
};
