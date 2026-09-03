#pragma once

#include "Shared/GuiPanel.h"

// The HTTP trigger server: its port, and a button per registered route so a
// trigger can be fired from inside the editor without an external HTTP client.
class GuiPanel_HttpTriggers : public GuiPanel
{
public:
	GuiPanel_HttpTriggers(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;

private:
	class ProjectGuiPanelContext* m_context= nullptr;
};
