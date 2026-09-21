#pragma once

#include "ObjectSystemFwd.h"
#include "ObjectFwd.h"
#include "MikanRendererFwd.h"

#include <functional>
#include <set>

// Decides whether an object's renderables are submitted. An empty filter submits every object.
using RenderableObjectFilter= std::function<bool(MikanObjectConstPtr)>;

void addAllRenderablesToMkScene(std::set<const MikanObjectSystem*> objectSystems, IMkScenePtr mkScene,
								RenderableObjectFilter objectFilter= {});

void addAllRenderablesToMkScene(MikanObjectSystemConstPtr objectSystem, IMkScenePtr mkScene,
								RenderableObjectFilter objectFilter= {});
