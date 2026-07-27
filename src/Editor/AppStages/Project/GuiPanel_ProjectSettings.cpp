#include "GuiPanel_ProjectSettings.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "AppStage.h"
#include "EditorObjectSystem.h"
#include "HttpTriggerWindow.h"
#include "LocalizationManager.h"
#include "MikanServer.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "Shared/GuiPanel_DMXObjectSystem.h"

#include "imgui.h"
#include <cstring>

bool GuiPanel_ProjectSettings::init(ProjectGuiPanelContext* context)
{
	m_context= context;
	AppStage_Project* ownerAppStage= context->getOwnerAppStage();
	m_editorSystem= ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();

	m_defaultGuiStyle= getGuiStyleManager()->getStyle("default_component_panel");

	LocalizationManager* locManager= ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLanguageId= locManager->getLanguage();
	m_languageIdList= locManager->getSupportedLanguages();
	m_languageDataSource.setEntries(m_languageIdList);

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
	if (ImGui::Checkbox("Render Origin", &renderOrigin))
	{
		addDeferredGuiEvent([this, renderOrigin]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderOriginFlag(renderOrigin); });
	}

	bool renderAnchors= editorConfig->getRenderAnchorsFlag();
	if (ImGui::Checkbox("Render Anchors", &renderAnchors))
	{
		addDeferredGuiEvent([this, renderAnchors]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderAnchorsFlag(renderAnchors); });
	}

	bool renderQuadStencils= editorConfig->getRenderQuadStencilsFlag();
	if (ImGui::Checkbox("Render Quad Stencils", &renderQuadStencils))
	{
		addDeferredGuiEvent(
			[this, renderQuadStencils]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderQuadStencilsFlag(renderQuadStencils); });
	}

	bool renderBoxStencils= editorConfig->getRenderBoxStencilsFlag();
	if (ImGui::Checkbox("Render Box Stencils", &renderBoxStencils))
	{
		addDeferredGuiEvent(
			[this, renderBoxStencils]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderBoxStencilsFlag(renderBoxStencils); });
	}

	bool renderModelStencils= editorConfig->getRenderModelStencilsFlag();
	if (ImGui::Checkbox("Render Model Stencils", &renderModelStencils))
	{
		addDeferredGuiEvent(
			[this, renderModelStencils]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setRenderModelStencilsFlag(renderModelStencils); });
	}

	int stencilDisplayIndex= (int)editorConfig->getModelStencilDisplayMode();
	const char* k_stencilDisplayLabels[]= {"Solid", "Wireframe", "Both"};
	if (ImGui::Combo("Model Stencil Display", &stencilDisplayIndex, k_stencilDisplayLabels,
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
		ImGui::SetTooltip("Wireframe hides the opaque mesh so the real model in the video shows through.");
	}

	ImGui::Separator();

	// Floor grid + ruler snapping
	ImGui::Text("Grid & Measurement");

	float gridExtent= editorConfig->getGridExtent();
	if (ImGui::InputFloat("Grid Extent (m)", &gridExtent, 0.5f, 1.f, "%.2f"))
	{
		if (gridExtent < 0.1f)
			gridExtent= 0.1f;
		addDeferredGuiEvent([this, gridExtent]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setGridExtent(gridExtent); });
	}

	float gridCellSize= editorConfig->getGridCellSize();
	if (ImGui::InputFloat("Grid Cell Size (m)", &gridCellSize, 0.05f, 0.1f, "%.3f"))
	{
		if (gridCellSize < 0.001f)
			gridCellSize= 0.001f;
		addDeferredGuiEvent([this, gridCellSize]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setGridCellSize(gridCellSize); });
	}

	float snapIncrement= editorConfig->getSnapIncrement();
	if (ImGui::InputFloat("Snap Increment (m)", &snapIncrement, 0.01f, 0.1f, "%.3f"))
	{
		if (snapIncrement < 0.f)
			snapIncrement= 0.f;
		addDeferredGuiEvent([this, snapIncrement]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setSnapIncrement(snapIncrement); });
	}

	bool snapEnabled= editorConfig->getSnapEnabled();
	if (ImGui::Checkbox("Ruler Snap To Grid By Default", &snapEnabled))
	{
		addDeferredGuiEvent([this, snapEnabled]()
							{ m_editorSystem.lock()->getEditorSystemConfig()->setSnapEnabled(snapEnabled); });
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Hold Shift while measuring to invert snapping.\n"
						  "Off (default): hold Shift to snap, release to move freely.");
	}

	int rulerUnitsIndex= (int)editorConfig->getRulerDisplayUnits();
	const char* k_rulerUnitLabels[]= {"Meters", "Centimeters", "Millimeters"};
	if (ImGui::Combo("Ruler Units", &rulerUnitsIndex, k_rulerUnitLabels, IM_ARRAYSIZE(k_rulerUnitLabels)))
	{
		addDeferredGuiEvent(
			[this, rulerUnitsIndex]()
			{
				m_editorSystem.lock()->getEditorSystemConfig()->setRulerDisplayUnits(
					(eRulerDisplayUnits)rulerUnitsIndex);
			});
	}

	bool debugCameraAlignment= editorConfig->getDebugCameraAlignment();
	if (ImGui::Checkbox("Debug Camera Alignment", &debugCameraAlignment))
	{
		addDeferredGuiEvent(
			[this, debugCameraAlignment]()
			{ m_editorSystem.lock()->getEditorSystemConfig()->setDebugCameraAlignment(debugCameraAlignment); });
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Overlays the MR alignment chain: axes at stage origin / puck / aperture,\n"
						  "and a window with live vs calibrated intrinsics, resolution, and offsets.");
	}

	ImGui::Separator();

	// Language selector
	AppStage_Project* ownerAppStage= m_context->getOwnerAppStage();
	LocalizationManager* locManager= ownerAppStage->getOwnerWindow()->getLocalizationManager();
	m_selectedLanguageId= locManager->getLanguage();

	m_languageDataSource.setEntries(m_languageIdList);
	int selectedIndex= m_languageDataSource.getEntryIndexByString(m_selectedLanguageId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectLanguage", "Language", &m_languageDataSource,
									selectedIndex))
	{
		if (selectedIndex >= 0)
		{
			const std::string lang= m_languageDataSource.getEntryDisplayString(selectedIndex);
			addDeferredGuiEvent([locManager, lang]() { locManager->setLanguage(lang); });
		}
	}

	ImGui::Separator();

	// Script editor command (e.g. "code --reuse-window", "devenv /edit", "" = OS default)
	{
		auto appSettings= App::getInstance()->getAppSettings();
		char editorBuf[256];
		strncpy_s(editorBuf, appSettings->getScriptEditorCommand().c_str(), sizeof(editorBuf) - 1);
		if (ImGui::InputText("Script Editor Command", editorBuf, sizeof(editorBuf)))
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
		if (ImGui::InputInt("HTTP Trigger Server Port", &httpPort))
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

		if (ImGui::Button("Show HTTP Triggers"))
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

	// DMX system settings
	m_context->getDMXSystemPanel()->onGui();
}
