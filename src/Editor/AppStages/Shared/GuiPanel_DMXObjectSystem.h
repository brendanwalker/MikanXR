#pragma once

#include "Shared/GuiPanel_MikanObjectSystem.h"
#include "IMkGuiStyle.h"
#include "LightSystemFwd.h"

class GuiPanel_DMXObjectSystem : public GuiPanel_MikanObjectSystem
{
public:
	GuiPanel_DMXObjectSystem(class AppStage* ownerAppStage)
		: GuiPanel_MikanObjectSystem(ownerAppStage)
	{
	}

	virtual bool init() override;
	virtual void onConstruct() override;
	virtual void onGui() override;

protected:
	// The universe -> unicast address table, drawn under the transport properties
	void drawDestinationTable();

	DMXObjectSystemPtr getDMXObjectSystem() const;
	DMXObjectSystemDefinitionPtr getDMXObjectSystemDefinition() const;

private:
	MkGuiStyleConstPtr m_defaultGuiStyle;
};
