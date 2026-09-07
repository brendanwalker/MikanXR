#pragma once

#include "LightSystemFwd.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "Shared/GuiPanel_MikanComponent.h"

class GuiPanel_DMXFixtureGroupComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_DMXFixtureGroupComponent(class AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	DMXFixtureGroupComponentPtr getDMXFixtureGroupComponent() const;

private:
	GuiDataSource_ComboBox m_stageDataSource;
	GuiDataSource_ComboBox m_fixtureDataSource;
	int m_selectedCandidateIndex= 0;
};
