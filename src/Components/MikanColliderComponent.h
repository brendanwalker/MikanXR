#pragma once

#include "MikanSceneComponent.h"
#include "IGlMesh.h"

#include <memory>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

struct ColliderRaycastHitResult
{
	float hitTime;
	glm::vec3 hitLocation;
};

class MikanColliderComponent : public MikanSceneComponent
{
public:
	MikanColliderComponent(MikanObjectWeakPtr owner);

	virtual bool hasRayOverlap(
		const glm::vec3 origin, const glm::vec3 direction,
		ColliderRaycastHitResult& outHitResult) const;
};
typedef std::weak_ptr<MikanColliderComponent> MikanColliderComponentWeakPtr;
typedef std::shared_ptr<MikanColliderComponent> MikanColliderComponentPtr;