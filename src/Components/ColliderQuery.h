#pragma once

#include "glm/ext/vector_float3.hpp"

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
