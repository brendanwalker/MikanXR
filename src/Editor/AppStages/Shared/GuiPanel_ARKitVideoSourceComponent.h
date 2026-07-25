#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "ARKitVideoSourceComponent.h"

// All of ARKitVideoSourceComponent's properties (basePort, depthStreamingEnabled,
// the JBU tuning floats) are plain int/bool/float - unlike
// GuiPanel_NetworkVideoSourceComponent's protocol dropdown, none of them need a
// custom IPropertyInterface renderer, so this panel only needs to wire up the
// typed property interface and a compact-mode summary.
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

protected:
	ARKitVideoSourceComponentPtr getARKitVideoSourceComponent() const;
};
