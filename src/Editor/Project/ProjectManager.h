#pragma once

#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"

#include <filesystem>
#include <string>
#include <vector>

class ProjectManager
{
public:
	ProjectManager(class IMkWindow* ownerWindow);

	inline ProjectConfigPtr getProjectConfig() const { return m_projectConfig; }
	inline class IMkWindow* getOwnerWindow() const { return m_ownerWindow; }

	template <class t_system_type>
	std::shared_ptr<t_system_type> addSystem() { 
		std::shared_ptr<t_system_type> systemPtr= std::make_shared<t_system_type>(this);
		m_systems.push_back(systemPtr); 

		return systemPtr;
	}

	template<class t_system_type>
	std::shared_ptr<t_system_type> getSystemOfType()
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
	void updateAutoSave(float deltaSeconds);

private:
	class IMkWindow* m_ownerWindow = nullptr;
	std::vector<MikanObjectSystemPtr> m_systems;

	// Project Config
	ProjectConfigPtr m_projectConfig;
	float m_projectSaveCooldown = -1.f;
};