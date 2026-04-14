#include "GuiPanel_ProjectSettings.h"
#include "AppStage.h"
#include "EditorObjectSystem.h"
#include "LocalizationManager.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"

#include "imgui.h"

bool GuiPanel_ProjectSettings::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	m_editorSystem = ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();

	m_defaultGuiStyle = getGuiStyleManager()->getStyle("default_component_panel");

	LocalizationManager* locManager = ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLanguageId = locManager->getLanguage();
	m_languageIdList = locManager->getSupportedLanguages();
	m_languageDataSource.setEntries(m_languageIdList);

	return true;
}

void GuiPanel_ProjectSettings::dispose()
{
	GuiPanel::dispose();
}

void GuiPanel_ProjectSettings::onGui()
{
	auto editorSystem = m_editorSystem.lock();
	if (!editorSystem)
		return;

	const auto editorConfig = editorSystem->getEditorSystemConfigConst();

	bool renderOrigin = editorConfig->getRenderOriginFlag();
	if (ImGui::Checkbox("Render Origin", &renderOrigin))
	{
		addDeferredGuiEvent([this, renderOrigin]() {
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderOriginFlag(renderOrigin);
		});
	}

	bool renderAnchors = editorConfig->getRenderAnchorsFlag();
	if (ImGui::Checkbox("Render Anchors", &renderAnchors))
	{
		addDeferredGuiEvent([this, renderAnchors]() {
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderAnchorsFlag(renderAnchors);
		});
	}

	bool renderQuadStencils = editorConfig->getRenderQuadStencilsFlag();
	if (ImGui::Checkbox("Render Quad Stencils", &renderQuadStencils))
	{
		addDeferredGuiEvent([this, renderQuadStencils]() {
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderQuadStencilsFlag(renderQuadStencils);
		});
	}

	bool renderBoxStencils = editorConfig->getRenderBoxStencilsFlag();
	if (ImGui::Checkbox("Render Box Stencils", &renderBoxStencils))
	{
		addDeferredGuiEvent([this, renderBoxStencils]() {
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderBoxStencilsFlag(renderBoxStencils);
		});
	}

	bool renderModelStencils = editorConfig->getRenderModelStencilsFlag();
	if (ImGui::Checkbox("Render Model Stencils", &renderModelStencils))
	{
		addDeferredGuiEvent([this, renderModelStencils]() {
			m_editorSystem.lock()->getEditorSystemConfig()->setRenderModelStencilsFlag(renderModelStencils);
		});
	}

	ImGui::Separator();

	// Language selector
	AppStage_Project* ownerAppStage = m_context->getOwnerAppStage();
	LocalizationManager* locManager = ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLanguageId = locManager->getLanguage();

	m_languageDataSource.setEntries(m_languageIdList);
	int selectedIndex = m_languageDataSource.getEntryIndexByString(m_selectedLanguageId);
	if (MkGui::drawComboBoxProperty(
		m_defaultGuiStyle, "projectLanguage", "Language",
		&m_languageDataSource, selectedIndex))
	{
		if (selectedIndex >= 0)
		{
			const std::string lang = m_languageDataSource.getEntryDisplayString(selectedIndex);
			addDeferredGuiEvent([locManager, lang]() {
				locManager->setLanguage(lang);
			});
		}
	}
}
