#pragma once

#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "ColliderQuery.h"

ColliderRaycastHitResult findClosestCollisionAlongRay(
	std::set<const MikanObjectSystem*> objectSystems,
	const ColliderRaycastHitRequest& request);

ColliderRaycastHitResult findClosestCollisionAlongRay(
	MikanObjectSystemConstPtr objectSystem,
	const ColliderRaycastHitRequest& request,
	const ColliderRaycastHitResult* inPrevClosestResult);