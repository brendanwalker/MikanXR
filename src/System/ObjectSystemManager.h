#pragma once

#include "ObjectSystemFwd.h"

#include <vector>

class ObjectSystemManager
{
public:
	template <class t_system_type>
	std::shared_ptr<t_system_type> addSystem() { 
		std::shared_ptr<t_system_type> systemPtr= std::make_shared<t_system_type>();
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
	
	void init();
	void dispose();
	void update();
	void customRender();

protected:
	std::vector<MikanObjectSystemPtr> m_systems;
};