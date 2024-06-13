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
		hitPriority= 0;
		hitLocation= glm::vec3(0.f);
		hitNormal= glm::vec3(0.f);
		hitComponent.reset();
		closestVertexLocal= glm::vec3(0.f);
		closestVertexWorld= glm::vec3(0.f);
		closestVertexValid= false;
	}
};
