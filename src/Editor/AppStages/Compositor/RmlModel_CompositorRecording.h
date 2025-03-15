#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlModel_CompositorRecording : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, const class GlFrameCompositor* compositor);
	virtual void dispose() override;

	const Rml::String& getVideoSourceName() const;
	void setVideoSourceName(const Rml::String& newName);

	const Rml::String& getVideoModeName() const;
	void setVideoModeName(const Rml::String& newName);

	eSupportedCodec getSelectedVideoCodec() const;
	void setSelectedVideoCodec(eSupportedCodec newCodec);

	bool getIsRecording() const;
	void setIsRecording(bool bNewFlag);

	bool getIsStreaming() const;
	void setIsStreaming(bool bNewFlag);

	SinglecastDelegate<void()> OnToggleRecordingEvent;
	SinglecastDelegate<void()> OnToggleStreamingEvent;
	SinglecastDelegate<void(eSupportedCodec)> OnVideoCodecChangedEvent;

protected:
	void onVideoFrameSizeChanged(const class VideoSourceView* videoSourceView);

private:
	VideoSourceViewPtr m_videoSource;

	// Recording UI
	Rml::String m_videoSourceName;
	bool m_bHasValidVideoSource= false;
	Rml::String m_videoModeName;
	Rml::Vector<Rml::String> m_videoCodecs;
	Rml::String m_selectedCodec;
	bool m_bIsRecording= false;
	bool m_bIsStreaming= false;
};
