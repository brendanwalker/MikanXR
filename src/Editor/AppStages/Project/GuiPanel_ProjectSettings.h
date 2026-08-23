#pragma once

#include "Shared/GuiPanel.h"
#include "Shared/GuiDataSource_StringList.h"
#include "EditorObjectSystem.h"
#include "MkGuiStyle.h"
#include "ObjectSystemFwd.h"

class GuiPanel_ProjectSettings : public GuiPanel
{
public:
	GuiPanel_ProjectSettings(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	class ProjectGuiPanelContext* m_context= nullptr;
	EditorObjectSystemWeakPtr m_editorSystem;

	std::string m_selectedLanguageId;
	std::vector<std::string> m_languageIdList;

	MkGuiStyleConstPtr m_defaultGuiStyle;
	GuiDataSource_StringList m_languageDataSource;
};
