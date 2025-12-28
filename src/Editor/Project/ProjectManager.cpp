#pragma once

#include "App.h"
#include "AppSettingsConfig.h"
#include "AnchorObjectSystem.h"
#include "ClientTextureSourceSystem.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "IMkWindow.h"
#include "MainWindow.h"
#include "MarkerObjectSystem.h"
#include "NetworkVideoSourceSystem.h"
#include "PathUtils.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "SceneObjectSystem.h"
#include "SpoutTextureSourceSystem.h"
#include "StencilObjectSystem.h"
#include "StageObjectSystem.h"
#include "TextureSourceSystem.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingVolumeObjectSystem.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSystem.h"
#include "VRObjectSystem.h"

#define PROJECT_SAVE_COOLDOWN	3.f

const char* ProjectManager::k_mikanProjectFileExtension= ".mikanproj";

ProjectManager::ProjectManager(IMkWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

bool ProjectManager::startup(MainWindow* mainWindow)
{
	// Allocate all systems, in the order we want to perform init and updates
	// Init EditorSystem first so that it get component creation events 
	// from Anchor and Stencil Systems triggered during init call
	addSystem<EditorObjectSystem>();
	addSystem<ClientTextureSourceSystem>();
	addSystem<SpoutTextureSourceSystem>();
	addSystem<NetworkVideoSourceSystem>();
	addSystem<USBVideoSourceSystem>();
	addSystem<TextureSourceSystem>();
	addSystem<VideoSourceSystem>();
	addSystem<MarkerObjectSystem>();
	addSystem<SceneObjectSystem>();
	addSystem<StageObjectSystem>();
	addSystem<CompositorObjectSystem>();
	addSystem<CameraObjectSystem>();
	addSystem<AnchorObjectSystem>();
	addSystem<StencilObjectSystem>();
	addSystem<VRObjectSystem>();
	addSystem<TrackingMountObjectSystem>();
	addSystem<TrackingVolumeObjectSystem>();

	// Create a default project if we can't load the last opened one
	AppSettingsConfigPtr appSettings= mainWindow->getOwnerApp()->getAppSettings();
	if (!loadProject(appSettings->getLastProjectPath().string()))
	{
		const std::filesystem::path defaultProjectPath =
			PathUtils::makeTimestampedFilePath(
				getDefaultProjectFolder(), "Default", k_mikanProjectFileExtension);

		if (newProject(defaultProjectPath.string()))
		{
			appSettings->setLastProjectPath(defaultProjectPath);
		}
		else
		{
			return false;
		}
	}

	return true;
}

void ProjectManager::shutdown()
{
	unloadProject();
	m_systems.clear();
}

void ProjectManager::update(float deltaSeconds)
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->update(deltaSeconds);
	}

	if (m_projectConfig)
	{
		m_projectConfig->updateAutoSave(deltaSeconds);
	}
}

void ProjectManager::customRender()
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->customRender();
	}
}

std::filesystem::path ProjectManager::getDefaultProjectFolder()
{
	return PathUtils::getHomeDirectory() / "Mikan";
}

bool ProjectManager::hasLoadedProject() const
{
	return m_projectConfig != nullptr;
}

bool ProjectManager::newProject(const std::string& projectFilePath)
{
	auto newProjectConfig = std::make_shared<ProjectConfig>();
	newProjectConfig->save(projectFilePath);

	return loadProject(projectFilePath);
}

bool ProjectManager::loadProject(const std::string& projectFilePath)
{
	// Early out if the project file path is invalid
	if (projectFilePath.empty() || !std::filesystem::exists(projectFilePath))
	{
		return false;
	}

	// Early out if the requested project path isn't new
	if (hasLoadedProject() && m_projectConfig->getLoadedConfigPath() == projectFilePath)
	{
		return true;
	}

	// Unload the existing project first
	unloadProject();

	// Attempt to load and init the new project
	m_projectConfig = std::make_shared<ProjectConfig>();		
	bool bSuccess = m_projectConfig->load(projectFilePath);
	if (bSuccess)
	{
		// Initialize all systems using the loaded project config
		for (int i = 0; i < (int)m_systems.size(); i++)
		{
			MikanObjectSystemPtr system = m_systems[i];

			if (!system->init())
			{
				bSuccess = false;
				break;
			}
		}
	}

	if (bSuccess)
	{
		// Enable auto-save
		m_projectConfig->setAutoSaveCooldownDuration(PROJECT_SAVE_COOLDOWN);
	}
	else
	{
		// Unload if we failed to load or init
		unloadProject();
	}

	return bSuccess;
}

bool ProjectManager::saveProject(const std::string& projectFilePath)
{
	if (hasLoadedProject())
	{
		m_projectConfig->save(projectFilePath);
		return true;
	}

	return false;
}

void ProjectManager::unloadProject()
{
	// Call dispose in reverse order
	// so that Editor system gets component destroy events
	// from the Anchor and Stencil Systems triggered during dispose call
	for (int i = (int)m_systems.size() - 1; i >= 0; i--)
	{
		MikanObjectSystemPtr system = m_systems[i];

		system->dispose();
	}

	m_projectConfig = nullptr;
}