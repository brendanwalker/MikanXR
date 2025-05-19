#include "RmlModel_CameraSettings.h"
#include "MathMikan.h"
#include "ProjectConfig.h"
#include "Shared/RmlDataBinding_CameraBrightness.h"
#include "StringUtils.h"
#include "VideoSourceManager.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CameraSettings::RmlModel_CameraSettings()
	: m_brightnessDataBinding(std::make_shared<RmlDataBinding_CameraBrightness>())
{
}

bool RmlModel_CameraSettings::init(
	Rml::Context* rmlContext,
	const ProjectConfigConstPtr profile,
	const VideoSourceManager* videoSourceManager)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "camera_settings");
	if (!constructor)
		return false;

	// Bind brightness to the data model
	if (!m_brightnessDataBinding->init(constructor))
		return false;

	// Register Data Model Fields
	constructor.Bind("selected_video_source", &m_videoSourcePath);
	constructor.Bind("video_sources", &m_videoSourcePathList);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"update_video_source",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const std::string devicePath = ev.GetParameter<Rml::String>("value", "");
			handleVideoSourcePathChanged(devicePath);
		});

	// Fill in the data model
	rebuildVideoSourceList(videoSourceManager);
	m_videoSourcePath = profile->videoSourcePath;

	return true;
}

void RmlModel_CameraSettings::dispose()
{
	m_brightnessDataBinding->dispose();
	OnUpdateVideoSourcePath.Clear();

	RmlModel::dispose();
}

void RmlModel_CameraSettings::rebuildVideoSourceList(const VideoSourceManager* videoSourceManager)
{
	VideoSourceList videoSourceList= videoSourceManager->getVideoSourceList();

	m_videoSourcePathList.clear();
	for (VideoSourceViewPtr videoSourceView : videoSourceList)
	{
		m_videoSourcePathList.push_back(videoSourceView->getUSBDevicePath());
	}
	m_modelHandle.DirtyVariable("video_sources");
}

void RmlModel_CameraSettings::handleVideoSourcePathChanged(const std::string& devicePath)
{
	if (devicePath != m_videoSourcePath)
	{
		m_videoSourcePath = devicePath;

		if (OnUpdateVideoSourcePath)
		{
			OnUpdateVideoSourcePath(devicePath);
		}
	}
}