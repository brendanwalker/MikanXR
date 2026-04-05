#pragma once

#include "Shared/GuiPanel.h"
#include "EditorObjectSystem.h"
#include "ObjectSystemFwd.h"

class GuiPanel_ProjectSettings : public GuiPanel
{
public:
	GuiPanel_ProjectSettings() = default;

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	class ProjectGuiPanelContext* m_context = nullptr;
	EditorObjectSystemWeakPtr m_editorSystem;

	std::string m_selectedLanguageId;
	std::vector<std::string> m_languageIdList;
};
