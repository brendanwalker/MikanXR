#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "CEFTextureSourceComponent.h"

class GuiPanel_CEFTextureSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_CEFTextureSourceComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	// -- IGuiPanel
	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	CEFTextureSourceComponentPtr getCEFTextureSourceComponent() const;
};
