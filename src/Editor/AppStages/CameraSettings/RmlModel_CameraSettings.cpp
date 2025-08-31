#include "RmlModel_CameraSettings.h"
#include "MathMikan.h"
#include "ProjectConfig.h"
#include "Shared/RmlDataBinding_CameraBrightness.h"
#include "StringUtils.h"
#include "VideoSourceSystem.h"
#include "VideoSourceComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CameraSettings::RmlModel_CameraSettings()
	: m_brightnessDataBinding(std::make_shared<RmlDataBinding_CameraBrightness>())
{
}

bool RmlModel_CameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceComponentPtr currentVideoSource)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "camera_settings");
	if (!constructor)
		return false;

	// Bind brightness to the data model
	if (!m_brightnessDataBinding->init(constructor))
		return false;

	// Register Data Model Fields
	constructor.Bind("selected_video_source", &m_videoSourceId);
	constructor.Bind("video_sources", &m_videoSourceIdList);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"update_video_source",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			MikanVideoSourceID videoSourceId = ev.GetParameter<int>("value", -1);
			handleVideoSourceIdChanged(videoSourceId);
		});

	// Fill in the data model
	rebuildVideoSourceList();

	// Set the current video source ID
	m_videoSourceId = currentVideoSource ? currentVideoSource->getVideoSourceId() : -1;

	return true;
}

void RmlModel_CameraSettings::dispose()
{
	m_brightnessDataBinding->dispose();
	OnUpdateVideoSourceId.Clear();

	RmlModel::dispose();
}

void RmlModel_CameraSettings::rebuildVideoSourceList()
{
	m_videoSourceIdList = VideoSourceSystem::getSystem()->getVideoSourceIdList();
	m_modelHandle.DirtyVariable("video_sources");
}

void RmlModel_CameraSettings::handleVideoSourceIdChanged(MikanVideoSourceID videoSourceId)
{
	if (videoSourceId != m_videoSourceId)
	{
		m_videoSourceId = videoSourceId;

		if (OnUpdateVideoSourceId)
		{
			OnUpdateVideoSourceId(videoSourceId);
		}
	}
}