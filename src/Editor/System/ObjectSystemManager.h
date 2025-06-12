#pragma once

#include "ObjectSystemFwd.h"

#include <vector>

class ObjectSystemManager
{
public:
	ObjectSystemManager(class IMkWindow* ownerWindow)
		: m_ownerWindow(ownerWindow)
	{}

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
	
	bool startup();
	void shutdown();
	void update(float deltaSeconds);
	void customRender();

protected:
	class IMkWindow* m_ownerWindow = nullptr;
	std::vector<MikanObjectSystemPtr> m_systems;
};