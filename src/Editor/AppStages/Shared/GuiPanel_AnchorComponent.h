#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "AnchorComponent.h"

class GuiPanel_AnchorComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_AnchorComponent(AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

private:
	GuiDataSource_ComboBox m_parentTransformDataSource;
};
