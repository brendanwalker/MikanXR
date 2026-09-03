#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "ARKitVideoSourceComponent.h"

// basePort is a plain int, rendered by the generic IPropertyInterface renderer
// without help - this class exists only for drawCompactGui()'s per-source-row
// summary used by the project outliner, not for any custom property rendering.
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
