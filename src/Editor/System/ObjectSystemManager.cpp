#pragma once

#include "ObjectSystemManager.h"
#include "AnchorObjectSystem.h"
#include "ClientVideoSourceSystem.h"
#include "CameraObjectSystem.h"
#include "EditorObjectSystem.h"
#include "IMkWindow.h"
#include "NetworkVideoSourceSystem.h"
#include "SceneObjectSystem.h"
#include "SpoutVideoSourceSystem.h"
#include "StencilObjectSystem.h"
#include "StageObjectSystem.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSystem.h"

bool ObjectSystemManager::startup()
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
	addSystem<SceneObjectSystem>();
	addSystem<StageObjectSystem>();
	addSystem<CameraObjectSystem>();
	addSystem<AnchorObjectSystem>();
	addSystem<StencilObjectSystem>();

	for (int i= 0; i < (int)m_systems.size(); i++)
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
	for (int i = (int)m_systems.size() - 1; i >= 0; i--)
	{
		MikanObjectSystemPtr system = m_systems[i];

		system->dispose();
	}
	m_systems.clear();
}

void ObjectSystemManager::update(float deltaSeconds)
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->update(deltaSeconds);
	}
}

void ObjectSystemManager::customRender()
{
	for (MikanObjectSystemPtr system : m_systems)
	{
		system->customRender();
	}
}
