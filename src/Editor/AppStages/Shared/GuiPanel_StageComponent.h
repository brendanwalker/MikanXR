#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "StageComponent.h"

class GuiPanel_StageComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_StageComponent(AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	StageComponentPtr getStageComponent() const;

private:
	GuiDataSource_ComboBox m_trackingVolumeDataSource;
};
