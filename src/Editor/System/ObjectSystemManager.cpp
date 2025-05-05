#pragma once

#include "ObjectSystemManager.h"
#include "AnchorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "StencilObjectSystem.h"

bool ObjectSystemManager::startup()
{
	// Allocate all systems, in the order we want to perform init and updates
	// Init EditorSystem first so that it get component creation events 
	// from Anchor and Stencil Systems triggered during init call
	addSystem<EditorObjectSystem>();
	addSystem<AnchorObjectSystem>();
	addSystem<StencilObjectSystem>();

	for (size_t i= 0; i < m_systems.size(); i++)
	{
		MikanObjectSystemPtr system = m_systems[i];

		if (!system->init())
		{
			return false;
		}
	}

	return true;
}

void ObjectSystemManager::shutdown()
{
	// Call dispose in reverse order 
	// so that Editor system gets component destroy events
	// from the Anchor and Stencil Systems triggered during dispose call
	for (size_t i = m_systems.size() - 1; i >= 0; i--)
	{
		MikanObjectSystemPtr system = m_systems[i];

		system->dispose();
	}
	m_systems.clear();
}

void ObjectSystemManager::update()
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->update();
	}
}

void ObjectSystemManager::customRender()
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->customRender();
	}
}
