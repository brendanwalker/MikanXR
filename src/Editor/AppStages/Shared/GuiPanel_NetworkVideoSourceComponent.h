#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_StringList.h"
#include "NetworkVideoSourceComponent.h"

class GuiPanel_NetworkVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_NetworkVideoSourceComponent(AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	void drawCompactGui();

	// -- GuiPanel_MikanComponent Interface
	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	NetworkVideoSourceComponentPtr getNetworkVideoSourceComponent() const;

private:
	GuiDataSource_StringList m_protocolDataSource;
};
