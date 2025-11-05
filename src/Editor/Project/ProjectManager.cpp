#pragma once

#include "AnchorObjectSystem.h"
#include "ClientVideoSourceSystem.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "IMkWindow.h"
#include "MarkerObjectSystem.h"
#include "NetworkVideoSourceSystem.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "SceneObjectSystem.h"
#include "SpoutVideoSourceSystem.h"
#include "StencilObjectSystem.h"
#include "StageObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "TrackingVolumeObjectSystem.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSystem.h"
#include "VRObjectSystem.h"

#define PROJECT_SAVE_COOLDOWN	3.f

ProjectManager::ProjectManager(IMkWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

bool ProjectManager::startup()
{
	// Allocate all systems, in the order we want to perform init and updates
	// Init EditorSystem first so that it get component creation events 
	// from Anchor and Stencil Systems triggered during init call
	addSystem<EditorObjectSystem>();
	addSystem<ClientVideoSourceSystem>();
	addSystem<NetworkVideoSourceSystem>();
	addSystem<SpoutVideoSourceSystem>();
	addSystem<USBVideoSourceSystem>();
	addSystem<VideoSourceSystem>();
	addSystem<MarkerObjectSystem>();
	addSystem<SceneObjectSystem>();
	addSystem<StageObjectSystem>();
	addSystem<CompositorObjectSystem>();
	addSystem<CameraObjectSystem>();
	addSystem<AnchorObjectSystem>();
	addSystem<StencilObjectSystem>();
	addSystem<TrackingMountObjectSystem>();
	addSystem<TrackingVolumeObjectSystem>();
	addSystem<VRObjectSystem>();

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

	updateAutoSave(deltaSeconds);
}

void ProjectManager::customRender()
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->customRender();
	}
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

	// Unload if we failed to load or init
	if (!bSuccess)
	{
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

void ProjectManager::updateAutoSave(float deltaSeconds)
{
	// We change the profile constantly as changes are made in the UI
	// Put the save to disk on a cooldown so we aren't writing to disk constantly
	if (m_projectSaveCooldown >= 0.f)
	{
		if (m_projectConfig->isMarkedDirty())
		{
			m_projectSaveCooldown -= deltaSeconds;
			if (m_projectSaveCooldown < 0.f)
			{
				m_projectConfig->save();
				m_projectSaveCooldown = -1.f;
			}
		}
		else
		{
			m_projectSaveCooldown = -1.f;
		}
	}
	else if (m_projectConfig->isMarkedDirty())
	{
		m_projectSaveCooldown = PROJECT_SAVE_COOLDOWN;
	}
}