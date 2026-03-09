#pragma once

#include "CommonConfigFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "MulticastDelegate.h"

#include <filesystem>
#include <string>
#include <vector>
#include <map>

using MikanPropertyDatabasePtr = std::shared_ptr<class MikanPropertyDatabase>;
using MikanPropertyDatabaseConstPtr = std::shared_ptr<const class MikanPropertyDatabase>;
using MikanFunctionDatabasePtr = std::shared_ptr<class MikanFunctionDatabase>;
using MikanFunctionDatabaseConstPtr = std::shared_ptr<const class MikanFunctionDatabase>;

class ProjectManager : public std::enable_shared_from_this<ProjectManager>
{
public:
	ProjectManager(class IMkWindow* ownerWindow);

	inline ProjectConfigPtr getProjectConfig() const { return m_projectConfig; }
	inline class IMkWindow* getOwnerWindow() const { return m_ownerWindow; }
	inline MikanPropertyDatabasePtr getPropertyDatabase() { return m_propertyDatabase; }
	inline MikanPropertyDatabaseConstPtr getPropertyDatabaseConst() const { return m_propertyDatabase; }
	inline MikanFunctionDatabasePtr getFunctionDatabase() { return m_functionDatabase; }
	inline MikanFunctionDatabaseConstPtr getFunctionDatabaseConst() const { return m_functionDatabase; }

	template <class t_system_type>
	std::shared_ptr<t_system_type> addSystem() { 
		std::shared_ptr<t_system_type> systemPtr= std::make_shared<t_system_type>(shared_from_this());
		registerSystem(systemPtr);

		return systemPtr;
	}

	template<class t_system_type>
	std::shared_ptr<t_system_type> getSystemOfType() const
	{
		for (MikanObjectSystemPtr system : m_systems)
		{
			std::shared_ptr<t_system_type> derivedSystem = std::dynamic_pointer_cast<t_system_type>(system);

			if (derivedSystem != nullptr)
			{
				return derivedSystem;
			}
		}

		return nullptr;
	}

	MikanObjectSystemPtr getSystemByName(const std::string name) const;
	
	bool startup(class MainWindow* mainWindow);
	void shutdown();
	void update(float deltaSeconds);
	void customRender();

	static std::filesystem::path getDefaultProjectFolder();
	static const char* k_mikanProjectFileExtension;

	bool hasLoadedProject() const;
	bool newProject(const std::string& projectFilePath);
	bool loadProject(const std::string& projectFilePath);
	bool saveProject(const std::string& projectFilePath);
	void unloadProject();

	MulticastDelegate<void(ProjectManagerPtr oldProject)> OnProjectPreUnload;
	MulticastDelegate<void(ProjectManagerPtr newProject)> OnProjectLoaded;

protected:
	void registerSystem(MikanObjectSystemPtr system);

private:
	class IMkWindow* m_ownerWindow = nullptr;
	std::vector<MikanObjectSystemPtr> m_systems;
	std::map<std::string, int> m_systemNameToIndexMap;
	MikanPropertyDatabasePtr m_propertyDatabase;
	MikanFunctionDatabasePtr m_functionDatabase;

	// Project Config
	ProjectConfigPtr m_projectConfig;
};