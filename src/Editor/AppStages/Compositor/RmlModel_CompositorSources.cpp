#include "RmlModel_CompositorSources.h"
#include "CompositorComponent.h"
#include "StringUtils.h"
#include "VideoSourceComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorSources::init(
	Rml::Context* rmlContext,
	CompositorComponentPtr compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_sources");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("video_source_name", &m_videoSourceName);
	constructor.Bind("has_valid_video_source", &m_bHasValidVideoSource);
	constructor.Bind("video_mode_name", &m_videoModeName);

	// Bind data model callbacks

	m_videoSource= compositor->getVideoSourceComponent();
	if (m_videoSource)
	{
		m_videoSourceName = m_videoSource->getName();
		m_videoSource->OnFrameSizeChanged += 
			MakeDelegate(this, &RmlModel_CompositorSources::onVideoFrameSizeChanged);
		onVideoFrameSizeChanged(m_videoSource);

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

void RmlModel_CompositorSources::dispose()
{
	if (m_videoSource)
	{
		m_videoSource->OnFrameSizeChanged -= 
			MakeDelegate(this, &RmlModel_CompositorSources::onVideoFrameSizeChanged);
	}

	RmlModel::dispose();
}

void RmlModel_CompositorSources::onVideoFrameSizeChanged(VideoSourceComponentPtr videoSourceComponent)
{
	if (!videoSourceComponent->getVideoModeName(m_videoModeName))
	{
		m_videoModeName = "INVALID";
	}
	m_modelHandle.DirtyVariable("video_mode_name");
}

const Rml::String& RmlModel_CompositorSources::getVideoSourceName() const
{
	return m_videoSourceName;
}

void RmlModel_CompositorSources::setVideoSourceName(const Rml::String& newName)
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

const Rml::String& RmlModel_CompositorSources::getVideoModeName() const
{
	return m_videoModeName;
}

void RmlModel_CompositorSources::setVideoModeName(const Rml::String& newName)
{
	if (newName != m_videoModeName)
	{
		m_videoModeName = newName;
		m_modelHandle.DirtyVariable("video_mode_name");
	}
}