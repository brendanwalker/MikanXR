#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlModel_CompositorSources : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	const Rml::String& getVideoSourceName() const;
	void setVideoSourceName(const Rml::String& newName);

	const Rml::String& getVideoModeName() const;
	void setVideoModeName(const Rml::String& newName);

protected:
	void onVideoFrameSizeChanged(const class VideoSourceView* videoSourceView);

private:
	VideoSourceViewPtr m_videoSource;

	// camera UI
	Rml::String m_videoSourceName;
	bool m_bHasValidVideoSource= false;
	Rml::String m_videoModeName;
};
