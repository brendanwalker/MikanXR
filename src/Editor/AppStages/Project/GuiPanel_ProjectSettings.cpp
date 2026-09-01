#include "GuiPanel_ProjectSettings.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "AppStage.h"
#include "EditorObjectSystem.h"
#include "HttpTriggerWindow.h"
#include "LocText.h"
#include "LocalizationManager.h"
#include "MikanServer.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "Shared/GuiPanel_DMXObjectSystem.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"
#include "SpoutLogRelay.h"

#include "imgui.h"
#include <algorithm>
#include <cstring>

bool GuiPanel_ProjectSettings::init(ProjectGuiPanelContext* context)
{
	m_context= context;
	AppStage_Project* ownerAppStage= context->getOwnerAppStage();
	m_editorSystem= ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();

	m_defaultGuiStyle= getGuiStyleManager()->getStyle("default_component_panel");

	LocalizationManager* locManager= ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLanguageId= locManager->getLanguage();

	m_languageIdList.clear();
	m_languageNameList.clear();
	for (const LocalizationManager::LanguageInfo& info : locManager->getSupportedLanguageInfos())
	{
		m_languageIdList.push_back(info.code);
		m_languageNameList.push_back(info.nativeName);
	}
	m_languageDataSource.setEntries(m_languageNameList);

	return true;
}

void GuiPanel_ProjectSettings::dispose() { GuiPanel::dispose(); }

void GuiPanel_ProjectSettings::onGui()
{
	auto editorSystem= m_editorSystem.lock();
	if (!editorSystem)
		return;

	const auto editorConfig= editorSystem->getEditorSystemConfigConst();

	bool renderOrigin= editorConfig->getRenderOriginFlag();
	if (ImGui::Checkbox(locLabel("projectSettings.renderOrigin"), &renderOrigin))
	{
		addDeferredGuiEvent([this, renderOrigin]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderOriginFlag(renderOrigin); });
	}

	bool renderAnchors= editorConfig->getRenderAnchorsFlag();
	if (ImGui::Checkbox(locLabel("projectSettings.renderAnchors"), &renderAnchors))
	{
		addDeferredGuiEvent([this, renderAnchors]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderAnchorsFlag(renderAnchors); });
	}

	bool renderQuadStencils= editorConfig->getRenderQuadStencilsFlag();
	if (ImGui::Checkbox(locLabel("projectSettings.renderQuadStencils"), &renderQuadStencils))
	{
		addDeferredGuiEvent(
			[this, renderQuadStencils]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderQuadStencilsFlag(renderQuadStencils); });
	}

	bool renderBoxStencils= editorConfig->getRenderBoxStencilsFlag();
	if (ImGui::Checkbox(locLabel("projectSettings.renderBoxStencils"), &renderBoxStencils))
	{
		addDeferredGuiEvent(
			[this, renderBoxStencils]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderBoxStencilsFlag(renderBoxStencils); });
	}

	bool renderModelStencils= editorConfig->getRenderModelStencilsFlag();
	if (ImGui::Checkbox(locLabel("projectSettings.renderModelStencils"), &renderModelStencils))
	{
		addDeferredGuiEvent(
			[this, renderModelStencils]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderModelStencilsFlag(renderModelStencils); });
	}

	int stencilDisplayIndex= (int)editorConfig->getModelStencilDisplayMode();
	const char* k_stencilDisplayLabels[]= {locText("projectSettings.stencilDisplaySolid"),
										   locText("projectSettings.stencilDisplayWireframe"),
										   locText("projectSettings.stencilDisplayBoth")};
	if (ImGui::Combo(locLabel("projectSettings.modelStencilDisplay"), &stencilDisplayIndex, k_stencilDisplayLabels,
					 IM_ARRAYSIZE(k_stencilDisplayLabels)))
	{
		addDeferredGuiEvent(
			[this, stencilDisplayIndex]()
			{
				m_editorSystem.lock()->getEditorSystemConfig()->setModelStencilDisplayMode(
					(eStencilDisplayMode)stencilDisplayIndex);
			});
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%s", locText("projectSettings.stencilDisplayTooltip"));
	}

	bool debugRenderInCompositor= editorConfig->getDebugRenderInCompositor();
	if (ImGui::Checkbox(locLabel("projectSettings.debugRenderInCompositor"), &debugRenderInCompositor))
	{
		addDeferredGuiEvent(
			[this, debugRenderInCompositor]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setDebugRenderInCompositor(debugRenderInCompositor); });
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%s", locText("projectSettings.debugRenderInCompositorTooltip"));
	}

	ImGui::Separator();

	// Floor grid + ruler snapping
	ImGui::TextUnformatted(locText("projectSettings.gridAndMeasurement"));

	float gridExtent= editorConfig->getGridExtent();
	if (ImGui::InputFloat(locLabel("projectSettings.gridExtent"), &gridExtent, 0.5f, 1.f, "%.2f"))
	{
		if (gridExtent < 0.1f)
			gridExtent= 0.1f;
		addDeferredGuiEvent([this, gridExtent]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setGridExtent(gridExtent); });
	}

	float gridCellSize= editorConfig->getGridCellSize();
	if (ImGui::InputFloat(locLabel("projectSettings.gridCellSize"), &gridCellSize, 0.05f, 0.1f, "%.3f"))
	{
		if (gridCellSize < 0.001f)
			gridCellSize= 0.001f;
		addDeferredGuiEvent([this, gridCellSize]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setGridCellSize(gridCellSize); });
	}

	float snapIncrement= editorConfig->getSnapIncrement();
	if (ImGui::InputFloat(locLabel("projectSettings.snapIncrement"), &snapIncrement, 0.01f, 0.1f, "%.3f"))
	{
		if (snapIncrement < 0.f)
			snapIncrement= 0.f;
		addDeferredGuiEvent([this, snapIncrement]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setSnapIncrement(snapIncrement); });
	}

	bool snapEnabled= editorConfig->getSnapEnabled();
	if (ImGui::Checkbox(locLabel("projectSettings.rulerSnapToGridByDefault"), &snapEnabled))
	{
		addDeferredGuiEvent([this, snapEnabled]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setSnapEnabled(snapEnabled); });
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%s", locText("projectSettings.rulerSnapTooltip"));
	}

	int rulerUnitsIndex= (int)editorConfig->getRulerDisplayUnits();
	const char* k_rulerUnitLabels[]= {locText("projectSettings.rulerUnitMeters"),
									  locText("projectSettings.rulerUnitCentimeters"),
									  locText("projectSettings.rulerUnitMillimeters")};
	if (ImGui::Combo(locLabel("projectSettings.rulerUnits"), &rulerUnitsIndex, k_rulerUnitLabels,
					 IM_ARRAYSIZE(k_rulerUnitLabels)))
	{
		addDeferredGuiEvent(
			[this, rulerUnitsIndex]()
			{
				m_editorSystem.lock()->getEditorSystemConfig()->setRulerDisplayUnits(
					(eRulerDisplayUnits)rulerUnitsIndex);
			});
	}

	bool debugCameraAlignment= editorConfig->getDebugCameraAlignment();
	if (ImGui::Checkbox(locLabel("projectSettings.debugCameraAlignment"), &debugCameraAlignment))
	{
		addDeferredGuiEvent(
			[this, debugCameraAlignment]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setDebugCameraAlignment(debugCameraAlignment); });
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("%s", locText("projectSettings.debugCameraAlignmentTooltip"));
	}

	ImGui::Separator();

	// Language selector
	AppStage_Project* ownerAppStage= m_context->getOwnerAppStage();
	LocalizationManager* locManager= ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLanguageId= locManager->getLanguage();

	m_languageDataSource.setEntries(m_languageNameList);
	const auto selectedIt= std::find(m_languageIdList.begin(), m_languageIdList.end(), m_selectedLanguageId);
	int selectedIndex= selectedIt != m_languageIdList.end() ? (int)(selectedIt - m_languageIdList.begin()) : -1;
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectLanguage", locText("projectSettings.language"),
									&m_languageDataSource, selectedIndex))
	{
		if (selectedIndex >= 0 && selectedIndex < (int)m_languageIdList.size())
		{
			const std::string lang= m_languageIdList[selectedIndex];
			addDeferredGuiEvent([locManager, lang]() { locManager->setLanguage(lang); });
		}
	}

	ImGui::Separator();

	// Script editor command (e.g. "code --reuse-window", "devenv /edit", "" = OS default)
	{
		auto appSettings= App::getInstance()->getAppSettings();
		char editorBuf[256];
		strncpy_s(editorBuf, appSettings->getScriptEditorCommand().c_str(), sizeof(editorBuf) - 1);
		if (ImGui::InputText(locLabel("projectSettings.scriptEditorCommand"), editorBuf, sizeof(editorBuf)))
		{
			const std::string newCmd(editorBuf);
			addDeferredGuiEvent([appSettings, newCmd]() { appSettings->setScriptEditorCommand(newCmd); });
		}
	}

	ImGui::Separator();

	// HTTP trigger server port (e.g. for Stream Deck style integrations)
	{
		auto appSettings= App::getInstance()->getAppSettings();
		int httpPort= appSettings->getHttpServerPort();
		if (ImGui::InputInt(locLabel("projectSettings.httpTriggerServerPort"), &httpPort))
		{
			if (httpPort < 1)
				httpPort= 1;
			if (httpPort > 65535)
				httpPort= 65535;

			addDeferredGuiEvent(
				[appSettings, httpPort]()
				{
					appSettings->setHttpServerPort(httpPort);
					MikanServer::getInstance()->restartHttpMessageServer(httpPort);
				});
		}

		if (ImGui::Button(locLabel("projectSettings.showHttpTriggers")))
		{
			addDeferredGuiEvent(
				[]()
				{
					App* app= App::getInstance();
					if (!app->hasWindowOfType<HttpTriggerWindow>())
						app->createAppWindow<HttpTriggerWindow>();
				});
		}
	}

	ImGui::Separator();

	// Spout log relay (Spout's own diagnostics, folded into the editor log)
	{
		auto appSettings= App::getInstance()->getAppSettings();
		bool spoutLogEnabled= appSettings->getSpoutLogEnabled();
		if (ImGui::Checkbox(locLabel("projectSettings.spoutLogging"), &spoutLogEnabled))
		{
			addDeferredGuiEvent(
				[appSettings, spoutLogEnabled]()
				{
					appSettings->setSpoutLogEnabled(spoutLogEnabled);
					App::getInstance()->getSpoutLogRelay()->setEnabled(spoutLogEnabled);
				});
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", locText("projectSettings.spoutLoggingTooltip"));
		}
	}

	ImGui::Separator();

	// DMX system settings
	m_context->getDMXSystemPanel()->onGui();

	ImGui::Separator();

	// Marker system settings
	m_context->getMarkerSystemPanel()->onGui();
}
