#pragma once

#include "IMkGuiStyle.h"
#include "ObjectSystemFwd.h"
#include "Shared/GuiPanel.h"

// Every scene in the project as a flat list. Clicking a row makes it the
// active scene and selects it in the outliner, where its properties show.
// The active scene draws with the outliner's active highlight style.
class GuiPanel_SceneList : public GuiPanel
{
public:
	GuiPanel_SceneList(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}

	bool init(class ProjectGuiPanelContext* context, class GuiPanel_ProjectOutliner* outliner);
	virtual void onGui() override;

private:
	class ProjectGuiPanelContext* m_context= nullptr;
	class GuiPanel_ProjectOutliner* m_outliner= nullptr;
	SceneObjectSystemWeakPtr m_sceneSystem;
	MkGuiStyleConstPtr m_activeRowStyle;
};
