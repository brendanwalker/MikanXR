#include "RmlModel_CompositorRecording.h"
#include "GlFrameCompositor.h"
#include "StringUtils.h"
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
	constructor.Bind("video_mode_name", &m_videoModeName);
	constructor.Bind("video_codecs", &m_videoCodecs);
	constructor.Bind("selected_codec", &m_selectedCodec);
	constructor.Bind("is_recording", &m_bIsRecording);
	constructor.Bind("is_streaming", &m_bIsStreaming);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"toggle_recording",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleRecordingEvent) OnToggleRecordingEvent();
		});
	constructor.BindEventCallback(
		"toggle_streaming",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleStreamingEvent) OnToggleStreamingEvent();
		});
	constructor.BindEventCallback(
		"changed_codec",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string codecString = ev.GetParameter<Rml::String>("value", "");
			const eSupportedCodec codec = 
				StringUtils::FindEnumValue<eSupportedCodec>(
					codecString, k_supportedCodecName);

			if (OnVideoCodecChangedEvent && codec != eSupportedCodec::INVALID)
			{
				OnVideoCodecChangedEvent(codec);
			}
		});

	// Set defaults
	for (int modeIndex = 0; modeIndex < (int)eSupportedCodec::COUNT; ++modeIndex)
	{
		m_videoCodecs.push_back(k_supportedCodecName[modeIndex]);
	}
	m_selectedCodec= k_supportedCodecName[int(eSupportedCodec::MP4V)];
	m_bIsRecording= false;
	m_bIsStreaming= false;

	m_videoSource= compositor->getVideoSource();
	if (m_videoSource)
	{
		m_videoSourceName = m_videoSource->getFriendlyName();
		m_videoSource->OnFrameSizeChanged += MakeDelegate(this, &RmlModel_CompositorRecording::onVideoFrameSizeChanged);
		onVideoFrameSizeChanged(m_videoSource.get());

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
	if (m_videoSource)
	{
		m_videoSource->OnFrameSizeChanged -= MakeDelegate(this, &RmlModel_CompositorRecording::onVideoFrameSizeChanged);
	}

	OnToggleRecordingEvent.Clear();
	OnVideoCodecChangedEvent.Clear();
	RmlModel::dispose();
}

void RmlModel_CompositorRecording::onVideoFrameSizeChanged(const VideoSourceView* videoSourceView)
{
	const VideoModeConfig* modeConfig= videoSourceView->getVideoMode();

	if (modeConfig != nullptr)
	{
		m_videoModeName = modeConfig->modeName;
	}
	else
	{
		m_videoModeName = "INVALID";
	}
	m_modelHandle.DirtyVariable("video_mode_name");
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
	return StringUtils::FindEnumValue<eSupportedCodec>(m_selectedCodec, k_supportedCodecName);
}

void RmlModel_CompositorRecording::setSelectedVideoCodec(eSupportedCodec newCodec)
{
	Rml::String newCodecString= k_supportedCodecName[int(newCodec)];
	if (newCodecString != m_selectedCodec)
	{
		m_selectedCodec = newCodecString;
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

bool RmlModel_CompositorRecording::getIsStreaming() const
{
	return m_bIsStreaming;
}

void RmlModel_CompositorRecording::setIsStreaming(bool bNewFlag)
{
	if (bNewFlag != m_bIsStreaming)
	{
		m_bIsStreaming = bNewFlag;
		m_modelHandle.DirtyVariable("is_streaming");
	}
}