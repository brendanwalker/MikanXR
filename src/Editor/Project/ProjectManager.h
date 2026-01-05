#pragma once

#include "CommonConfigFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"

#include <filesystem>
#include <string>
#include <vector>
#include <map>

using MikanPropertyDatabasePtr = std::shared_ptr<class MikanPropertyDatabase>;
using MikanPropertyDatabaseConstPtr = std::shared_ptr<const class MikanPropertyDatabase>;

class ProjectManager
{
public:
	ProjectManager(class IMkWindow* ownerWindow);

	inline ProjectConfigPtr getProjectConfig() const { return m_projectConfig; }
	inline class IMkWindow* getOwnerWindow() const { return m_ownerWindow; }
	inline MikanPropertyDatabasePtr getPropertyDatabase() { return m_propertyDatabase; }
	inline MikanPropertyDatabaseConstPtr getPropertyDatabaseConst() const { return m_propertyDatabase; }

	template <class t_system_type>
	std::shared_ptr<t_system_type> addSystem() { 
		std::shared_ptr<t_system_type> systemPtr= std::make_shared<t_system_type>(this);
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

protected:
	void registerSystem(MikanObjectSystemPtr system);

private:
	class IMkWindow* m_ownerWindow = nullptr;
	std::vector<MikanObjectSystemPtr> m_systems;
	std::map<std::string, int> m_systemNameToIndexMap;
	MikanPropertyDatabasePtr m_propertyDatabase;

	// Project Config
	ProjectConfigPtr m_projectConfig;
};