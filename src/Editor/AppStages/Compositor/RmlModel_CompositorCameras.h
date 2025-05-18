#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlModel_CompositorCameras : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	//const Rml::String& getVideoSourceName() const;
	//void setVideoSourceName(const Rml::String& newName);

private:
	ProjectConfigPtr m_projectConfigPtr;
	VideoSourceViewPtr m_videoSource;

	// Cameras UI
	Rml::Vector<Rml::String> m_cameraNames;
};
