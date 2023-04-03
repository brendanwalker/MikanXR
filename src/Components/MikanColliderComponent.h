#pragma once

#include "MikanSceneComponent.h"
#include "IGlMesh.h"

#include <memory>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

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

class MikanColliderComponent : public MikanSceneComponent
{
public:
	MikanColliderComponent(MikanObjectWeakPtr owner);

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const;
};
typedef std::weak_ptr<MikanColliderComponent> MikanColliderComponentWeakPtr;
typedef std::shared_ptr<MikanColliderComponent> MikanColliderComponentPtr;