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
	glm::vec3 hitLocation;
	glm::vec3 hitNormal;
	ColliderComponentWeakPtr hitComponent;

	ColliderRaycastHitResult()
	{
		hitValid= false;
		hitDistance= -1.f;
		hitLocation= glm::vec3(0.f);
		hitNormal= glm::vec3(0.f);
		hitComponent.reset();
	}
};
