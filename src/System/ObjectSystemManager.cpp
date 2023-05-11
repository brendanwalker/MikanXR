#pragma once

#include "ObjectSystemManager.h"
#include "AnchorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "FastenerObjectSystem.h"
#include "StencilObjectSystem.h"

void ObjectSystemManager::init()
{
	addSystem<AnchorObjectSystem>();
	addSystem<StencilObjectSystem>();
	addSystem<FastenerObjectSystem>();
	addSystem<EditorObjectSystem>();

	for (MikanObjectSystemPtr system : m_systems)
	{
		system->init();
	}
}

void ObjectSystemManager::dispose()
{
	for (MikanObjectSystemPtr system : m_systems)
	{
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
