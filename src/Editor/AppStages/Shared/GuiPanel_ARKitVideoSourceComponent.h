#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "ARKitVideoSourceComponent.h"

// basePort/depthStreamingEnabled are plain int/bool, rendered by the generic
// IPropertyInterface renderer without help. The JBU tuning floats (sigmaSpatial/
// sigmaColor/confWeightLow/confWeightMedium) get custom slider renderers below,
// bounded to sensible ranges instead of the generic renderer's unbounded drag-float
// - these are meant to be swept in real time while watching the depth preview
// (AppStage_VideoSourceSettings) for flicker/edge quality, so a bounded slider is a
// much better fit than a free-typed field. jbu_radius (int) has no slider
// equivalent in MkGuiDrawUtils, so it stays on the generic int renderer; its setter
// (ARKitVideoSourceDefinition::setJbuRadius) still clamps to a safe range.
class GuiPanel_ARKitVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_ARKitVideoSourceComponent(AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	void drawCompactGui();

	// -- GuiPanel_MikanComponent Interface
	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	ARKitVideoSourceComponentPtr getARKitVideoSourceComponent() const;
};
