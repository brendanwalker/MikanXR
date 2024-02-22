#include "RmlModel_DepthMeshCameraSettings.h"
#include "Constants_DepthMeshCapture.h"
#include "ProfileConfig.h"
#include "StringUtils.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceInterface.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_DepthMeshCameraSettings::init(
	Rml::Context* rmlContext,
	VideoSourceViewConstPtr videoSourceView,
	ProfileConfigConstPtr profileConfig)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "depth_mesh_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("menu_state", &m_menuState);
	constructor.Bind("viewpoint_mode", &m_viewpointMode);
	constructor.BindEventCallback(
		"viewpoint_mode_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (ev.GetId() == Rml::EventId::Change)
			{
				const std::string modeName = ev.GetParameter<Rml::String>("value", "");

				// A non-empty "value" means this check box was changed to the checked state
				if (!modeName.empty())
				{
					auto mode = StringUtils::FindEnumValue<eDepthMeshCaptureViewpointMode>(
						modeName, k_DepthMeshCaptureViewpointModeStrings);

					if (OnViewpointModeChanged)
					{
						OnViewpointModeChanged(mode);
					}
				}
			}
		});

	// Set defaults
	setMenuState(eDepthMeshCaptureMenuState::inactive);
	setViewpointMode(eDepthMeshCaptureViewpointMode::cameraViewpoint);

	return true;
}

void RmlModel_DepthMeshCameraSettings::dispose()
{

	OnViewpointModeChanged.Clear();
	RmlModel::dispose();
}

eDepthMeshCaptureMenuState RmlModel_DepthMeshCameraSettings::getMenuState() const
{
	return StringUtils::FindEnumValue<eDepthMeshCaptureMenuState>(
		m_menuState, k_DepthMeshCaptureMenuStateStrings);
}

void RmlModel_DepthMeshCameraSettings::setMenuState(eDepthMeshCaptureMenuState newState)
{
	Rml::String newStateString = k_DepthMeshCaptureMenuStateStrings[(int)newState];

	if (m_menuState != newStateString)
	{
		// Update menu state on the data model
		m_menuState = newStateString;
		m_modelHandle.DirtyVariable("menu_state");
	}
}

eDepthMeshCaptureViewpointMode RmlModel_DepthMeshCameraSettings::getViewpointMode() const
{
	return StringUtils::FindEnumValue<eDepthMeshCaptureViewpointMode>(
		m_viewpointMode, k_DepthMeshCaptureViewpointModeStrings);
}

void RmlModel_DepthMeshCameraSettings::setViewpointMode(eDepthMeshCaptureViewpointMode newMode)
{
	Rml::String newModeString = k_DepthMeshCaptureViewpointModeStrings[(int)newMode];

	if (m_viewpointMode != newModeString)
	{
		// Update menu state on the data model
		m_viewpointMode = newModeString;
		m_modelHandle.DirtyVariable("viewpoint_mode");
	}
}