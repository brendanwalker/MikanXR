//-- inludes -----
#include "IMkGraphicsContext.h"
#include "IMkShaderCache.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkScopedState.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "IMkState.h"
#include "IMkTriangulatedMesh.h"
#include "Project/AppStage_Project.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "MkGuiScopedWindow.h"
#include "ProjectManager.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "MainWindow.h"
#include "PathUtils.h"
#include "Logger.h"

#include "imgui.h"
#include "tinyfiledialogs.h"

//-- statics ----
const char* AppStage_MainMenu::APP_STAGE_NAME= "MainMenu";

//-- public methods -----
AppStage_MainMenu::AppStage_MainMenu(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_MainMenu::APP_STAGE_NAME)
{
}

void AppStage_MainMenu::enter()
{
	AppStage::enter();

	App* app= App::getInstance();
	m_appSettingsConfig= app->getAppSettings();
	m_projectManager= app->getMainWindow()->getProjectManager();

	// Create the background quad
	if (!m_fullscreenRGBQuad)
	{
		MkMaterialConstPtr backgroundMaterial=
			getOwnerWindow()->getGraphicsContext()->getShaderCache()->getMaterialByName(
				INTERNAL_MATERIAL_PT_PM5544_TEST_CARD);

		m_fullscreenRGBQuad=
			createFullscreenQuadMesh(getOwnerWindow()->getGraphicsContext().get(), backgroundMaterial, false);
	}
}

void AppStage_MainMenu::onResumeProject()
{
	std::vector<std::string> outResults;
	handleResumeProjectCommand(outResults);
}

void AppStage_MainMenu::onOpenProject()
{
	std::string defaultPath= (PathUtils::getProjectsRootDirectory() / "").string();
	static const char* filterItems[1]= {"*.mikanproj"};

	const char* picked=
		tinyfd_openFileDialog("Open Project", defaultPath.c_str(), 1, filterItems, "Project Files (*.mikanproj)", 1);

	if (picked == nullptr || picked[0] == '\0')
		return;

	std::filesystem::path projectFilePath(picked);

	std::vector<std::string> parameters= {projectFilePath.string()};
	std::vector<std::string> outResults;
	handleOpenProjectCommand(parameters, outResults);
}

void AppStage_MainMenu::onNewProject()
{
	std::string defaultPath= (PathUtils::getProjectsRootDirectory() / "").string();

	const char* picked= tinyfd_selectFolderDialog("New Project Folder", defaultPath.c_str());

	if (picked == nullptr || picked[0] == '\0')
		return;

	std::filesystem::path projectFolderPath(picked);
	std::string projectFileName = 
		projectFolderPath.filename().string() + std::string(ProjectManager::k_mikanProjectFileExtension);
	std::filesystem::path projectFilePath = std::filesystem::path(projectFolderPath) / projectFileName;

	std::vector<std::string> parameters= { projectFilePath.string() };
	std::vector<std::string> outResults;
	handleNewProjectCommand(parameters, outResults);
}

void AppStage_MainMenu::onExit()
{
	std::vector<std::string> outResults;
	handleExitCommand(outResults);
}

void AppStage_MainMenu::onGui()
{
	AppStage::onGui();

	constexpr float k_panelWidth= 300.f;
	const ImVec2 center= ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(k_panelWidth, 0), ImGuiCond_Always);

	constexpr ImGuiWindowFlags k_flags=
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

	MkGuiScopedWindow panel("##MainMenu", nullptr, k_flags);
	if (!panel)
		return;

	const float buttonWidth= k_panelWidth - 40.f;
	ImGui::SetCursorPosX((k_panelWidth - buttonWidth) * 0.5f);
	ImGui::Text("Main Menu");
	ImGui::Separator();
	ImGui::Spacing();

	if (m_appSettingsConfig->hasLastProjectPath())
	{
		ImGui::SetCursorPosX((k_panelWidth - buttonWidth) * 0.5f);
		if (ImGui::Button("Resume Project", ImVec2(buttonWidth, 0)))
			onResumeProject();
	}
	ImGui::SetCursorPosX((k_panelWidth - buttonWidth) * 0.5f);
	if (ImGui::Button("Open Project", ImVec2(buttonWidth, 0)))
		onOpenProject();
	ImGui::SetCursorPosX((k_panelWidth - buttonWidth) * 0.5f);
	if (ImGui::Button("New Project", ImVec2(buttonWidth, 0)))
		onNewProject();
	ImGui::SetCursorPosX((k_panelWidth - buttonWidth) * 0.5f);
	if (ImGui::Button("Exit", ImVec2(buttonWidth, 0)))
		onExit();
}

void AppStage_MainMenu::render(IMkViewportPtr targetViewport)
{
	AppStage::render(targetViewport);

	if (m_fullscreenRGBQuad)
	{
		MkMaterialInstancePtr materialInstance= m_fullscreenRGBQuad->getMaterialInstance();
		MkMaterialConstPtr material= materialInstance->getMaterial();

		if (auto materialBinding= material->bindMaterial())
		{
			// TODO: "Time" and "ScreenSize" are uniforms that all materials
			//  should have available by default in the graphics context
			const double currentTimeSeconds= m_ownerWindow->getOwnerApp()->getSecondsSinceAppStart();
			const float shaderTime= (float)fmodf(currentTimeSeconds, 1000.0);
			const glm::vec2 screenSize(m_ownerWindow->getWidth(), m_ownerWindow->getHeight());
			materialInstance->setVec2BySemantic(eUniformSemantic::screenSize, screenSize);
			materialInstance->setFloatBySemantic(eUniformSemantic::floatConstant0, shaderTime);

			if (auto materialInstanceBinding= materialInstance->bindMaterialInstance(materialBinding))
			{
				MkScopedState scopedState= getOwnerWindow()->getGraphicsContext()->getMkStateStack().createScopedState(
					"MainTargetDepthRender");
				IMkState* mkState= scopedState.getStackState();

				mkState->disableFlag(eMkStateFlagType::depthTest);
				mkState->disableFlag(eMkStateFlagType::cullFace);

				m_fullscreenRGBQuad->drawElements();
			}
		}
	}
}

// -- IRemoteControllableAppStage Interface -- //
bool AppStage_MainMenu::handleRemoteControlCommand(const std::string& command,
												   const std::vector<std::string>& parameters,
												   std::vector<std::string>& outResults)
{
	if (command == "resume_project")
	{
		return handleResumeProjectCommand(outResults);
	}
	else if (command == "open_project")
	{
		return handleOpenProjectCommand(parameters, outResults);
	}
	else if (command == "new_project")
	{
		return handleNewProjectCommand(parameters, outResults);
	}
	else if (command == "exit")
	{
		return handleExitCommand(outResults);
	}

	return AppStage::handleRemoteControlCommand(command, parameters, outResults);
}

bool AppStage_MainMenu::handleResumeProjectCommand(std::vector<std::string>& outResults)
{
	if (m_projectManager->hasLoadedProject())
	{
		m_ownerWindow->pushAppStageOfType<AppStage_Project>();
		outResults.push_back(IRemoteControllable::k_success);
	}
	else if (m_appSettingsConfig->hasLastProjectPath())
	{
		std::filesystem::path projectFilePath= m_appSettingsConfig->getLastProjectPath();

		if (m_projectManager->loadProject(projectFilePath.string()))
		{
			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
			outResults.push_back(IRemoteControllable::k_success);
		}
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return true;
}

bool AppStage_MainMenu::handleOpenProjectCommand(const std::vector<std::string>& parameters,
												 std::vector<std::string>& outResults)
{
	std::string projectFilePathStr= !parameters.empty() ? parameters[0] : "";

	if (!projectFilePathStr.empty())
	{
		if (m_projectManager->loadProject(projectFilePathStr))
		{
			// Remember the last opened project path
			m_appSettingsConfig->setLastProjectPath(projectFilePathStr);

			m_ownerWindow->pushAppStageOfType<AppStage_Project>();

			outResults.push_back(IRemoteControllable::k_success);
			return true;
		}
		else
		{
			outResults.push_back(IRemoteControllable::k_failure);
			return true;
		}
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return false;
}

bool AppStage_MainMenu::handleNewProjectCommand(const std::vector<std::string>& parameters,
												std::vector<std::string>& outResults)
{
	if (!parameters.empty())
	{
		const std::string& projectFilePathStr = parameters[0];

		if (m_projectManager->newProject(projectFilePathStr))
		{
			// Remember the last opened project path
			m_appSettingsConfig->setLastProjectPath(projectFilePathStr);

			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
			outResults.push_back(IRemoteControllable::k_success);
		}
		else
		{
			outResults.push_back(IRemoteControllable::k_failure);
		}

		return true;
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return false;
}

bool AppStage_MainMenu::handleExitCommand(std::vector<std::string>& outResults)
{
	App::getInstance()->requestShutdown();
	outResults.push_back(IRemoteControllable::k_success);

	return true;
}