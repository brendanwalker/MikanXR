#include "App.h"
#include "ProfileConfig.h"
#include "StencilObjectSystem.h"

StencilObjectSystem::StencilObjectSystem() : MikanObjectSystem()
{}

void StencilObjectSystem::init()
{
	MikanObjectSystem::init();

	m_objects.clear();
}