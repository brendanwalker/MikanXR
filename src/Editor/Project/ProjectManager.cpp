#pragma once

#include "App.h"
#include "AppSettingsConfig.h"
#include "AnchorObjectSystem.h"
#include "ARKitVideoSourceSystem.h"
#include "ClientTextureSourceSystem.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "DMXObjectSystem.h"
#include "EditorObjectSystem.h"
#include "PropertyInterface.h"
#include "MainWindow.h"
#include "MarkerObjectSystem.h"
#include "MikanPropertyDatabase.h"
#include "MikanFunctionDatabase.h"
#include "NetworkVideoSourceSystem.h"
#include "PathUtils.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "SceneObjectSystem.h"
#include "SpoutTextureSourceSystem.h"
#include "CEFTextureSourceSystem.h"
#include "BoxShapeSystem.h"
#include "BoxStencilSystem.h"
#include "ModelShapeSystem.h"
#include "ModelStencilSystem.h"
#include "QuadShapeSystem.h"
#include "QuadStencilSystem.h"
#include "RGBSpotLightSystem.h"
#include "RGBPixelGridSystem.h"
#include "StageObjectSystem.h"
#include "TrackingMountObjectSystem.h"
#include "MarkerTrackingVolumeSystem.h"
#include "VRTrackingVolumeSystem.h"
#include "USBVideoSourceSystem.h"
#include "VRObjectSystem.h"

#include "Logger.h"

#include <chrono>
#include <easy/profiler.h>

#define PROJECT_SAVE_COOLDOWN 3.f

const char* ProjectManager::k_mikanProjectFileExtension= ".mikanproj";

ProjectManager::ProjectManager(IEditorWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
	, m_propertyDatabase(std::make_shared<MikanPropertyDatabase>())
	, m_functionDatabase(std::make_shared<MikanFunctionDatabase>())
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
	addSystem<CEFTextureSourceSystem>();
	addSystem<NetworkVideoSourceSystem>();
	addSystem<USBVideoSourceSystem>();
	addSystem<ARKitVideoSourceSystem>();
	addSystem<MarkerObjectSystem>();
	addSystem<StageObjectSystem>();
	addSystem<SceneObjectSystem>();
	addSystem<CompositorObjectSystem>();
	addSystem<CameraObjectSystem>();
	addSystem<AnchorObjectSystem>();
	addSystem<QuadShapeSystem>();
	addSystem<BoxShapeSystem>();
	addSystem<ModelShapeSystem>();
	addSystem<QuadStencilSystem>();
	addSystem<BoxStencilSystem>();
	addSystem<ModelStencilSystem>();
	addSystem<VRObjectSystem>();
	addSystem<TrackingMountObjectSystem>();
	addSystem<MarkerTrackingVolumeSystem>();
	addSystem<VRTrackingVolumeSystem>();
	addSystem<DMXObjectSystem>();
	addSystem<RGBSpotLightSystem>();
	addSystem<RGBPixelGridSystem>();

	// Gather all property descriptors from all the systems and add them to the database
	for (int i= 0; i < (int)m_systems.size(); i++)
	{
		m_systems[i]->registerPropertyDescriptors(m_propertyDatabase);
	}

	// Gather all function descriptors from all the systems and add them to the database
	for (int i= 0; i < (int)m_systems.size(); i++)
	{
		m_systems[i]->registerFunctionDescriptors(m_functionDatabase);
	}

	// Create a default project if we can't load the last opened one
	AppSettingsConfigPtr appSettings= mainWindow->getOwnerApp()->getAppSettings();
	if (!loadProject(appSettings->getLastProjectPath().string()))
	{
		const std::string defaultProjectFolderName= PathUtils::makeTimestampedFileName("Default", "");
		const std::filesystem::path defaultProjectFilename= defaultProjectFolderName + k_mikanProjectFileExtension;
		const std::filesystem::path defaultProjectPath=
			getDefaultProjectFolder() / defaultProjectFolderName / defaultProjectFilename;

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

ProjectConfigPtr ProjectManager::createEmptyProjectConfig() const
{
	return std::make_shared<ProjectConfig>("ProfileConfig");
}

void ProjectManager::registerSystem(MikanObjectSystemPtr system)
{
	const int systemIndex= (int)m_systems.size();

	m_systemNameToIndexMap.insert({system->getObjectSystemClassName(), systemIndex});
	m_systems.push_back(system);
}

MikanObjectSystemPtr ProjectManager::getSystemByName(const std::string name) const
{
	auto it= m_systemNameToIndexMap.find(name);
	if (it != m_systemNameToIndexMap.end())
	{
		const int systemIndex= it->second;
		assert(systemIndex >= 0 && systemIndex < (int)m_systems.size());

		return m_systems[systemIndex];
	}

	return MikanObjectSystemPtr();
}

MikanComponentPtr ProjectManager::getComponentById(MikanComponentID componentId) const
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		MikanComponentPtr componentPtr= system->getComponentById(componentId);
		if (componentPtr)
		{
			return componentPtr;
		}
	}
	return MikanComponentPtr();
}

void ProjectManager::shutdown()
{
	unloadProject();
	m_systems.clear();
	m_propertyDatabase->clear();
	m_functionDatabase->clear();
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

std::filesystem::path ProjectManager::getDefaultProjectFolder() { return PathUtils::getProjectsRootDirectory(); }

bool ProjectManager::hasLoadedProject() const { return m_projectConfig != nullptr; }

bool ProjectManager::isAnySystemLoading() const
{
	for (const MikanObjectSystemPtr& system : m_systems)
	{
		if (system->isLoading())
			return true;
	}
	return false;
}

bool ProjectManager::newProject(const std::string& projectFilePath)
{
	// Determine the project directory (parent of the .mikanproj file)
	std::filesystem::path projectDir= std::filesystem::path(projectFilePath).parent_path();

	// Create the project directory if it doesn't exist
	if (!std::filesystem::exists(projectDir))
	{
		std::filesystem::create_directories(projectDir);
	}

	// Copy bundled resources (models, scripts, graphs, shaders, textures) into the project folder
	const std::filesystem::path resourcesDir= PathUtils::getResourceDirectory();
	const std::vector<std::string> projectResourceFolders= {"models", "scripts", "graphs", "shaders", "textures"};

	for (const std::string& folder : projectResourceFolders)
	{
		std::filesystem::path srcDir= resourcesDir / folder;
		std::filesystem::path dstDir= projectDir / folder;

		if (std::filesystem::exists(srcDir))
		{
			std::filesystem::copy(srcDir, dstDir,
								  std::filesystem::copy_options::recursive
									  | std::filesystem::copy_options::update_existing);
		}
	}

	auto newProjectConfig= createEmptyProjectConfig();
	newProjectConfig->save(projectFilePath);

	return loadProject(projectFilePath);
}

bool ProjectManager::loadProject(const std::string& projectFilePath)
{
	EASY_FUNCTION();

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

	// Set the project directory so PathUtils can resolve project-relative resources
	PathUtils::setProjectDirectory(std::filesystem::path(projectFilePath).parent_path());

	// create an empty project config
	m_projectConfig= createEmptyProjectConfig();

	// Attempt to load and init the new project
	auto loadStart= std::chrono::high_resolution_clock::now();
	bool bSuccess= m_projectConfig->load(projectFilePath);
	auto loadEnd= std::chrono::high_resolution_clock::now();
	MIKAN_LOG_INFO("ProjectManager::loadProject")
		<< "Config load: " << std::chrono::duration_cast<std::chrono::milliseconds>(loadEnd - loadStart).count()
		<< "ms";

	if (bSuccess)
	{
		// Initialize all systems using the loaded project config
		for (int i= 0; i < (int)m_systems.size(); i++)
		{
			MikanObjectSystemPtr system= m_systems[i];

			auto initStart= std::chrono::high_resolution_clock::now();
			MikanObjectSystemDefinitionPtr systemDefinition= m_projectConfig->getDefinitionForSystem(system);

			if (!system->init(systemDefinition))
			{
				bSuccess= false;
				break;
			}
			auto initEnd= std::chrono::high_resolution_clock::now();
			MIKAN_LOG_INFO("ProjectManager::loadProject")
				<< system->getObjectSystemClassName()
				<< "::init: " << std::chrono::duration_cast<std::chrono::milliseconds>(initEnd - initStart).count()
				<< "ms";
		}

		// Once all systems are successfully initialized,
		// call postInit on all systems to allow them to perform any setup
		// that requires other systems/components to be initialized
		// (e.g. TransformComponent wants to attach to its parent in postInit)
		auto postInitStart= std::chrono::high_resolution_clock::now();
		for (int i= 0; i < (int)m_systems.size(); i++)
		{
			m_systems[i]->postInit();
		}
		auto postInitEnd= std::chrono::high_resolution_clock::now();
		MIKAN_LOG_INFO("ProjectManager::loadProject")
			<< "postInit (all systems): "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(postInitEnd - postInitStart).count() << "ms";
	}

	if (bSuccess)
	{
		// Broadcast that the project has been loaded
		if (OnProjectLoaded)
		{
			OnProjectLoaded(shared_from_this());
		}

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
	// Broadcast that the project is about to be unloaded
	if (OnProjectPreUnload)
	{
		OnProjectPreUnload(shared_from_this());
	}

	// Call dispose in reverse order
	// so that Editor system gets component destroy events
	// from the Anchor and Stencil Systems triggered during dispose call
	for (int i= (int)m_systems.size() - 1; i >= 0; i--)
	{
		MikanObjectSystemPtr system= m_systems[i];

		system->dispose();
	}

	// Clear the current project directory so PathUtils doesn't resolve project-relative resources
	PathUtils::setProjectDirectory(std::filesystem::path());

	m_projectConfig= nullptr;
}