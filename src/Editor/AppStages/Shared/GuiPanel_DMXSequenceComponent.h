#pragma once

#include "LightSystemFwd.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "Shared/GuiPanel_MikanComponent.h"

class GuiPanel_DMXSequenceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_DMXSequenceComponent(class AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	DMXSequenceComponentPtr getDMXSequenceComponent() const;

private:
	GuiDataSource_ComboBox m_groupDataSource;
};
