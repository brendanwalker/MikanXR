#pragma once

#include "ObjectSystemFwd.h"
#include "ObjectFwd.h"
#include "MikanRendererFwd.h"
#include "ColliderQuery.h"

#include <functional>
#include <set>

// Decides whether an object's colliders are tested. An empty filter tests every object.
using ColliderObjectFilter= std::function<bool(MikanObjectConstPtr)>;

ColliderRaycastHitResult findClosestCollisionAlongRay(std::set<const MikanObjectSystem*> objectSystems,
													  const ColliderRaycastHitRequest& request,
													  ColliderObjectFilter objectFilter= {});

ColliderRaycastHitResult findClosestCollisionAlongRay(MikanObjectSystemConstPtr objectSystem,
													  const ColliderRaycastHitRequest& request,
													  const ColliderRaycastHitResult* inPrevClosestResult,
													  ColliderObjectFilter objectFilter= {});
