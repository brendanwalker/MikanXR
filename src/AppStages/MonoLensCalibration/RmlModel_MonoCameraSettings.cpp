#include "RmlModel_MonoCameraSettings.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_MonoCameraSettings::init(
	Rml::Context* rmlContext)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "mono_camera_settings");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("video_display_mode", &m_videoDisplayMode);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"viewpoint_mode_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
		if (ev.GetId() == Rml::EventId::Change)
		{
			const std::string modeName = ev.GetParameter<Rml::String>("value", "");

			// A non-empty "value" means this check box was changed to the checked state
			if (!modeName.empty())
			{
				auto mode = StringUtils::FindEnumValue<eVideoDisplayMode>(modeName, k_videoDisplayModeStrings);

				if (OnVideoDisplayModeChanged)
				{
					OnVideoDisplayModeChanged(mode);
				}
			}
		}
	});

	// Set defaults
	m_videoDisplayMode = k_videoDisplayModeStrings[(int)eVideoDisplayMode::mode_bgr];

	return true;
}

void RmlModel_MonoCameraSettings::setVideoDisplayMode(eVideoDisplayMode newMode, bool bMarkDirty)
{
	Rml::String newModeString= k_videoDisplayModeStrings[(int)newMode];

	if (newModeString != m_videoDisplayMode)
	{
		m_videoDisplayMode= newModeString;

		m_modelHandle.DirtyVariable("video_display_mode");
	}
}