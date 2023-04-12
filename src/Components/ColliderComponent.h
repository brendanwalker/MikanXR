#pragma once

#include "SceneComponent.h"
#include "IGlMesh.h"
#include "ObjectFwd.h"

#include <memory>

struct ColliderRaycastHitRequest
{
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
};

struct ColliderRaycastHitResult
{
	float hitDistance;
	glm::vec3 hitLocation;
	glm::vec3 hitNormal;
};

class ColliderComponent : public SceneComponent
{
public:
	ColliderComponent(MikanObjectWeakPtr owner);

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const;
};