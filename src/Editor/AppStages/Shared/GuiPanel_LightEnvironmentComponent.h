#pragma once

#include "LightSystemFwd.h"
#include "Shared/GuiPanel_MikanComponent.h"

/// Property panel for a scene lighting probe.
///
/// Everything except the component name is recovered by the capture tool
/// rather than authored, so the descriptors are read only and this panel
/// exists mainly to present them usefully: the 27 spherical harmonic floats
/// are drawn as color swatches instead of a wall of numbers.
class GuiPanel_LightEnvironmentComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_LightEnvironmentComponent(class AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	LightEnvironmentComponentPtr getLightEnvironmentComponent() const;
};
