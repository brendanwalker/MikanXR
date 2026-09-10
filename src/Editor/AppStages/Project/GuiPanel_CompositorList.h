#pragma once

#include "IMkGuiStyle.h"
#include "ObjectSystemFwd.h"
#include "Shared/GuiPanel.h"

// The active scene's compositors as a flat list. Clicking a row makes it the
// scene's display compositor and selects it in the outliner, where its
// properties show. The display compositor draws with the outliner's active
// highlight style.
class GuiPanel_CompositorList : public GuiPanel
{
public:
	GuiPanel_CompositorList(AppStage* ownerAppStage)
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
