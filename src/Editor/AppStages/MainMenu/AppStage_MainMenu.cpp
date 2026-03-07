//-- inludes -----
#include "IMkShaderCode.h"
#include "IMkShaderCache.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkScopedState.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "IMkState.h"
#include "IMkTextureCache.h"
#include "IMkTexture.h"
#include "IMkTriangulatedMesh.h"
#include "Project/AppStage_Project.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "MainMenu/RmlModel_MainMenu.h"
#include "ProjectManager.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "MainWindow.h"
#include "PathUtils.h"
#include "Logger.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

#include "tinyfiledialogs.h"

//-- statics ----
const char* AppStage_MainMenu::APP_STAGE_NAME = "MainMenu";

#define TIME_UNIFORM_NAME				"time"
#define SCREEN_SIZE_UNIFORM_NAME		"screenSize"
#define MENU_TEXTURE_UNIFORM_NAME		"rgbaTexture"

namespace MainMenuGfx {
	static const char* vertex_shader =
		R""""(
		#version 330 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTexCoords;

		out vec2 TexCoords;

		void main()
		{
			TexCoords = aTexCoords;
			gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
		}
		)"""";

	static const char* procedural_fragment_shader =
		R""""(
		#version 330 core
		out vec4 FragColor;

		in vec2 TexCoords;

		uniform vec2 screenSize;
		uniform float time;
		uniform sampler2D rgbaTexture;

		void main()
		{
			// Scroll speed (adjust these for desired speed)
			float scrollSpeedX = 0.05; // negative = left
			float scrollSpeedY = 0.05;  // positive = down

			// Create scrolling coordinates
			vec2 scrollPos = TexCoords;
			scrollPos.x += time * scrollSpeedX;
			scrollPos.y += time * scrollSpeedY;

			// Rotate coordinates to align with diagonal
			float angle = radians(-45.0); // -45 degrees for diagonal
			mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
			vec2 rotatedPos = rotation * scrollPos;

			// Scale for capsule dimensions
			float capsuleFrequency = 32.0;  // Number of capsules across screen
			float capsuleAspect = 3.0;     // Length to width ratio
			rotatedPos.x *= capsuleFrequency;
			rotatedPos.y *= capsuleFrequency / capsuleAspect;

			// Create repeating capsule pattern using sine waves
			float alongCapsule = sin(rotatedPos.x);
			float acrossCapsule = sin(rotatedPos.y * 10.0); // 2.0 for alternating rows

			// Combine to create capsule shapes
			float capsulePattern = alongCapsule * 0.7 + acrossCapsule * 0.3;

			// Smooth the capsule edges
			float capsule = smoothstep(-0.4, 0.4, capsulePattern);

			// Orange color
			vec3 orangeColor = vec3(1.0, 0.5, 0.0);
			vec3 backgroundColor = vec3(0.1, 0.1, 0.15); // Dark background

			// Mix between background and orange
			vec3 col = mix(backgroundColor, orangeColor, capsule);

			FragColor = vec4(col, 1.0);
		} 
		)"""";

	static const char* fragment_shader =
		R""""(
		#version 330 core
		out vec4 FragColor;

		in vec2 TexCoords;

		uniform vec2 screenSize;
		uniform float time;
		uniform sampler2D rgbaTexture;

		void main()
		{
			// Calculate aspect ratio
			float aspectRatio = screenSize.x / screenSize.y;

			// Scale factor (1/20th of screen)
			float scale = 15.0;

			// Scroll speed (adjust these for desired speed)
			float scrollSpeedX = -0.1; // negative = left
			float scrollSpeedY = 0.1;  // positive = down

			// Apply aspect ratio correction and scaling
			vec2 uv = TexCoords;
			uv.y = 1.0 - uv.y; // Flip vertical
			uv.x *= aspectRatio;
			uv *= scale;

			// Add scrolling offset based on time
			uv.x -= mod(time * scrollSpeedX, 1.f);
			uv.y -= mod(time * scrollSpeedY, 1.f);

			vec4 col = texture(rgbaTexture, uv);
			FragColor = col;
		} 
		)"""";

	static IMkShaderCodePtr getTextureProgramCode()
	{
		static IMkShaderCodePtr background_shader_code = nullptr;

		if (background_shader_code == nullptr)
		{
			background_shader_code = createIMkShaderCode(
				"MainMenu Background Shader",
				// vertex shader`
				vertex_shader,
				//fragment shader
				fragment_shader);
			background_shader_code->addVertexAttribute("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position);
			background_shader_code->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			background_shader_code->addUniform(SCREEN_SIZE_UNIFORM_NAME, eUniformSemantic::screenSize);
			background_shader_code->addUniform(TIME_UNIFORM_NAME, eUniformSemantic::floatConstant0);
			background_shader_code->addUniform(MENU_TEXTURE_UNIFORM_NAME, eUniformSemantic::rgbaTexture);
		}

		return background_shader_code;
	}
}

//-- public methods -----
AppStage_MainMenu::AppStage_MainMenu(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_MainMenu::APP_STAGE_NAME)
{ 
}

void AppStage_MainMenu::enter()
{
	AppStage::enter();

	App* app = App::getInstance();
	m_appSettingsConfig= app->getAppSettings();
	m_projectManager = app->getMainWindow()->getProjectManager();

	// Create the background quad
	if (!m_fullscreenRGBQuad)
	{
		// Create a shader for rendering the background
		IMkShaderCodePtr backgroundShaderCode = MainMenuGfx::getTextureProgramCode();
		IMkShaderPtr backgroundShader =
			getOwnerWindow()->getShaderCache()->fetchCompiledIMkShader(backgroundShaderCode);
		auto backgroundMaterial = std::make_shared<MkMaterial>("MainMenuBackground", backgroundShader);

		// Create a quad to render the background on
		m_fullscreenRGBQuad =
			createFullscreenQuadMesh(
				getOwnerWindow(),
				backgroundMaterial,
				false);
	}

	// Load the background texture
	if (!m_backgroundTexture)
	{
		m_backgroundTexture = getOwnerWindow()->getTextureCache()->loadTexturePath(
			PathUtils::getResourceDirectory() / "icons" / "mikan_icon.png");

		// Set the texture to repeat for the scrolling background effect
		if (m_backgroundTexture)
		{
			m_backgroundTexture->setTextureWrapMode(IMkTexture::TextureWrapMode::Repeat);
			m_backgroundTexture->reloadTextureFromImagePath();
		}
	}

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		auto* mainMenuModel = addRmlModel<RmlModel_MainMenu>();
		mainMenuModel->init(context, m_appSettingsConfig);
		mainMenuModel->OnResumeProject = MakeDelegate(this, &AppStage_MainMenu::onResumeProject);
		mainMenuModel->OnOpenProject = MakeDelegate(this, &AppStage_MainMenu::onOpenProject);
		mainMenuModel->OnNewProject = MakeDelegate(this, &AppStage_MainMenu::onNewProject);
		mainMenuModel->OnTutorial = MakeDelegate(this, &AppStage_MainMenu::onTutorial);
		mainMenuModel->OnExit = MakeDelegate(this, &AppStage_MainMenu::onExit);

		// Init calibration view now that the dependent model has been created
		addRmlDocument("main_menu.rml");
	}
}

void AppStage_MainMenu::onResumeProject()
{
	std::vector<std::string> outResults;
	handleResumeProjectCommand(outResults);
}

void AppStage_MainMenu::onOpenProject()
{
	std::string defaultPath = (PathUtils::getHomeDirectory() / "").string();
	static const char* filterItems[1] = { "*.mikanproj" };
	std::filesystem::path projectFilePath =
		tinyfd_openFileDialog(
			"Open Project",
			defaultPath.c_str(),
			1,
			filterItems,
			"Project Files (*.mikanproj)",
			1);

	std::vector<std::string> parameters = { projectFilePath.string() };
	std::vector<std::string> outResults;
	handleOpenProjectCommand(parameters, outResults);
}

void AppStage_MainMenu::onNewProject()
{
	std::string defaultPath = (PathUtils::getHomeDirectory() / "").string();
	static const char* filterItems[1] = { "*.mikanproj" };
	std::filesystem::path projectFilePath =
		tinyfd_saveFileDialog(
			"New Project",
			defaultPath.c_str(),
			1,
			filterItems,
			"Project Files (*.mikanproj)");

	std::vector<std::string> parameters = { projectFilePath.string() };
	std::vector<std::string> outResults;
	handleNewProjectCommand(parameters, outResults);
}

void AppStage_MainMenu::onTutorial()
{
	std::vector<std::string> outResults;
	handleTutorialCommand(outResults);
}

void AppStage_MainMenu::onExit()
{
	std::vector<std::string> outResults;
	handleExitCommand(outResults);
}

void AppStage_MainMenu::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	m_shaderTime += deltaSeconds;
}

void AppStage_MainMenu::render(IMkViewportPtr targetViewport)
{
	AppStage::render(targetViewport);

	if (m_backgroundTexture && m_fullscreenRGBQuad)
	{
		MkMaterialInstancePtr materialInstance = m_fullscreenRGBQuad->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			// Get the screen dimensions
			const float screenWidth = m_ownerWindow->getWidth();
			const float screenHeight = m_ownerWindow->getHeight();
			const glm::vec2 screenSize(screenWidth, screenHeight);

			// Bind the color texture
			materialInstance->setTextureByUniformName(MENU_TEXTURE_UNIFORM_NAME, m_backgroundTexture);
			materialInstance->setFloatByUniformName(TIME_UNIFORM_NAME, m_shaderTime);
			materialInstance->setVec2ByUniformName(SCREEN_SIZE_UNIFORM_NAME, screenSize);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				MkScopedState scopedState =
					getOwnerWindow()->getMkStateStack().createScopedState("MainTargetDepthRender");
				IMkState* mkState= scopedState.getStackState();

				mkState->disableFlag(eMkStateFlagType::depthTest);
				mkState->disableFlag(eMkStateFlagType::cullFace);
				mkState->enableFlag(eMkStateFlagType::texture2d);
				mkState->enableFlag(eMkStateFlagType::blend);

				//mkStateSetClearColor(mkState, glm::vec4(0.f, 0.f, 0.f, 1.f));
				//mkStateSetColorMask(mkState, glm::bvec4(true, true, true, true));

				mkStateSetBlendEquation(mkState, eMkBlendEquation::ADD);
				mkStateSetBlendFunc(mkState, eMkBlendFunction::SRC_ALPHA, eMkBlendFunction::ONE_MINUS_SRC_ALPHA);

				m_fullscreenRGBQuad->drawElements();
			}
		}
	}
}

// -- IRemoteControllableAppStage Interface -- //
bool AppStage_MainMenu::handleRemoteControlCommand(
	const std::string& command,
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
	else if (command == "tutorial")
	{
		return handleTutorialCommand(outResults);
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
		std::filesystem::path projectFilePath = m_appSettingsConfig->getLastProjectPath();

		if (m_projectManager->loadProject(projectFilePath.string()))
		{
			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
			outResults.push_back(IRemoteControllable::k_success);
		}
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return true;
}

bool AppStage_MainMenu::handleOpenProjectCommand(
	const std::vector<std::string>& parameters, 
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

bool AppStage_MainMenu::handleNewProjectCommand(
	const std::vector<std::string>& parameters,
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

bool AppStage_MainMenu::handleTutorialCommand(std::vector<std::string>& outResults)
{
	//TODO
	outResults.push_back(IRemoteControllable::k_failure);
	return false;
}

bool AppStage_MainMenu::handleExitCommand(std::vector<std::string>& outResults)
{
	App::getInstance()->requestShutdown();
	outResults.push_back(IRemoteControllable::k_success);

	return true;
}