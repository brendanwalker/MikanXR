#pragma once

#include "LightSystemFwd.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "Shared/GuiPanel_MikanComponent.h"

class GuiPanel_DMXPresetComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_DMXPresetComponent(class AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	DMXPresetComponentPtr getDMXPresetComponent() const;

private:
	GuiDataSource_ComboBox m_groupDataSource;
};
