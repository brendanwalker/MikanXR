#include "RmlModel_CompositorRecording.h"
#include "GlFrameCompositor.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorRecording::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_recording");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("video_source_name", &m_videoSourceName);
	constructor.Bind("has_valid_video_source", &m_bHasValidVideoSource);
	constructor.Bind("video_mode_name", &m_videoSourceName);
	constructor.Bind("video_codecs", &m_videoCodecs);
	constructor.Bind("selected_codec", &m_selectedCodec);
	constructor.Bind("is_recording", &m_bIsRecording);

	// Bind data model callbacks

	// Set defaults
	m_videoCodecs= {"MP4V", "MJPG", "RGBA"};
	m_selectedCodec= int(eSupportedCodec::SUPPORTED_CODEC_MP4V);
	m_bIsRecording= false;

	VideoSourceViewPtr videoSource= compositor->getVideoSource();
	if (videoSource)
	{
		m_videoSourceName = videoSource->getFriendlyName();
		m_videoModeName = videoSource->getVideoMode()->modeName;
		m_bHasValidVideoSource= true;
	}
	else
	{
		m_videoSourceName = "No Video Source";
		m_videoSourceName = "INVALID";
		m_bHasValidVideoSource= false;
	}

	return true;
}

void RmlModel_CompositorRecording::dispose()
{
	OnToggleRecordingEvent.Clear();
	OnVideoCodecChangedEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorRecording::update()
{
	if (m_modelHandle.IsVariableDirty("selected_codec"))
	{
		if (OnVideoCodecChangedEvent) OnVideoCodecChangedEvent(eSupportedCodec(m_selectedCodec));
	}
}

const Rml::String& RmlModel_CompositorRecording::getVideoSourceName() const
{
	return m_videoSourceName;
}

void RmlModel_CompositorRecording::setVideoSourceName(const Rml::String& newName)
{
	if (newName != m_videoSourceName)
	{
		m_videoSourceName = newName;
		m_modelHandle.DirtyVariable("video_source_name");

		bool bNewValidSource= m_videoSourceName.size() > 0;
		if (bNewValidSource != m_bHasValidVideoSource)
		{
			m_bHasValidVideoSource= bNewValidSource;
			m_modelHandle.DirtyVariable("has_valid_video_source");
		}
	}
}

const Rml::String& RmlModel_CompositorRecording::getVideoModeName() const
{
	return m_videoModeName;
}

void RmlModel_CompositorRecording::setVideoModeName(const Rml::String& newName)
{
	if (newName != m_videoModeName)
	{
		m_videoModeName = newName;
		m_modelHandle.DirtyVariable("video_mode_name");
	}
}

eSupportedCodec RmlModel_CompositorRecording::getSelectedVideoCodec() const
{
	return eSupportedCodec(m_selectedCodec);
}

void RmlModel_CompositorRecording::setSelectedVideoCodec(eSupportedCodec newCodec)
{
	int newCodecIndex= int(newCodec);
	if (newCodecIndex != m_selectedCodec)
	{
		m_selectedCodec = newCodecIndex;
		m_modelHandle.DirtyVariable("selected_codec");
	}
}

bool RmlModel_CompositorRecording::getIsRecording() const
{
	return m_bIsRecording;
}

void RmlModel_CompositorRecording::setIsRecording(bool bNewFlag)
{
	if (bNewFlag != m_bIsRecording)
	{
		m_bIsRecording= bNewFlag;
		m_modelHandle.DirtyVariable("is_recording");
	}
}