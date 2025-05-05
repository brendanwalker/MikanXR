#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlModel_CompositorVideo : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	const Rml::String& getVideoSourceName() const;
	void setVideoSourceName(const Rml::String& newName);

	const Rml::String& getVideoModeName() const;
	void setVideoModeName(const Rml::String& newName);

	bool getIsStreaming() const;
	void setIsStreaming(bool bNewFlag);

	SinglecastDelegate<void()> OnToggleStreamingEvent;

protected:
	void onVideoFrameSizeChanged(const class VideoSourceView* videoSourceView);

private:
	VideoSourceViewPtr m_videoSource;

	// Video UI
	Rml::String m_videoSourceName;
	bool m_bHasValidVideoSource= false;
	Rml::String m_videoModeName;
	bool m_bIsStreaming= false;
};
