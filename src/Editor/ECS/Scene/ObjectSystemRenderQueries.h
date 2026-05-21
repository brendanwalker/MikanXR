#pragma once

#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"

#include <set>

void addAllRenderablesToMkScene(
	std::set<const MikanObjectSystem*> objectSystems,
	IMkScenePtr mkScene);

void addAllRenderablesToMkScene(
	MikanObjectSystemConstPtr objectSystem,
	IMkScenePtr mkScene);