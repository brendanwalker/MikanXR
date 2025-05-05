#include "RmlModel_CompositorVideo.h"
#include "GlFrameCompositor.h"
#include "StringUtils.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorVideo::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_video");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("video_source_name", &m_videoSourceName);
	constructor.Bind("has_valid_video_source", &m_bHasValidVideoSource);
	constructor.Bind("video_mode_name", &m_videoModeName);
	constructor.Bind("is_streaming", &m_bIsStreaming);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"toggle_streaming",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnToggleStreamingEvent) OnToggleStreamingEvent();
		});

	// Set defaults
	m_bIsStreaming= false;

	m_videoSource= compositor->getVideoSource();
	if (m_videoSource)
	{
		m_videoSourceName = m_videoSource->getFriendlyName();
		m_videoSource->OnFrameSizeChanged += MakeDelegate(this, &RmlModel_CompositorVideo::onVideoFrameSizeChanged);
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

void RmlModel_CompositorVideo::dispose()
{
	if (m_videoSource)
	{
		m_videoSource->OnFrameSizeChanged -= MakeDelegate(this, &RmlModel_CompositorVideo::onVideoFrameSizeChanged);
	}

	RmlModel::dispose();
}

void RmlModel_CompositorVideo::onVideoFrameSizeChanged(const VideoSourceView* videoSourceView)
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

const Rml::String& RmlModel_CompositorVideo::getVideoSourceName() const
{
	return m_videoSourceName;
}

void RmlModel_CompositorVideo::setVideoSourceName(const Rml::String& newName)
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

const Rml::String& RmlModel_CompositorVideo::getVideoModeName() const
{
	return m_videoModeName;
}

void RmlModel_CompositorVideo::setVideoModeName(const Rml::String& newName)
{
	if (newName != m_videoModeName)
	{
		m_videoModeName = newName;
		m_modelHandle.DirtyVariable("video_mode_name");
	}
}

bool RmlModel_CompositorVideo::getIsStreaming() const
{
	return m_bIsStreaming;
}

void RmlModel_CompositorVideo::setIsStreaming(bool bNewFlag)
{
	if (bNewFlag != m_bIsStreaming)
	{
		m_bIsStreaming = bNewFlag;
		m_modelHandle.DirtyVariable("is_streaming");
	}
}