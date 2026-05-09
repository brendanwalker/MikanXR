#pragma once

#include "ComponentFwd.h"
#include "glm/ext/vector_float3.hpp"

struct ColliderRaycastHitRequest
{
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
};

struct ColliderRaycastHitResult
{
	bool hitValid;
	float hitDistance;
	int hitPriority;
	glm::vec3 hitLocation;
	glm::vec3 hitNormal;
	ColliderComponentWeakPtr hitComponent;
	glm::vec3 closestVertexLocal;
	glm::vec3 closestVertexWorld;
	bool closestVertexValid;

	ColliderRaycastHitResult()
	{
		hitValid= false;
		hitDistance= -1.f;
		hitPriority= -1;
		hitLocation= glm::vec3(0.f);
		hitNormal= glm::vec3(0.f);
		hitComponent.reset();
		closestVertexLocal= glm::vec3(0.f);
		closestVertexWorld= glm::vec3(0.f);
		closestVertexValid= false;
	}

	const bool isHigherPriorityThan(const ColliderRaycastHitResult& other) const
	{
		if (!this->hitValid)
			return false;

		if (!other.hitValid)
			return true;

		if (this->hitPriority < other.hitPriority)
			return false;

		if (this->hitPriority > other.hitPriority)
			return true;

		return this->hitDistance < other.hitDistance;
	}
};
