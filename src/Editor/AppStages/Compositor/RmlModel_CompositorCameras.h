#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlDataBinding_VRDeviceList;
using RmlDataBinding_VRDeviceListPtr = std::shared_ptr<RmlDataBinding_VRDeviceList>;

class RmlModel_CompositorCameras : public RmlModel
{
public:
	RmlModel_CompositorCameras();

	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	//const Rml::String& getVideoSourceName() const;
	//void setVideoSourceName(const Rml::String& newName);

private:
	RmlDataBinding_VRDeviceListPtr m_vrDeviceBinding;
	ProjectConfigPtr m_projectConfigPtr;
	VideoSourceViewPtr m_videoSource;

	// Cameras UI
	Rml::String m_cameraVRDevicePath;
	Rml::Vector<Rml::String> m_cameraNames;
};
