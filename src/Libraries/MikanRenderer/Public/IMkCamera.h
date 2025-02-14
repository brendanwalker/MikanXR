#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

#include <string>

class IMkCamera
{
public:
	virtual ~IMkCamera() {};

	virtual void setName(const std::string& name) = 0;
	virtual const std::string& getName() const = 0;

	virtual const glm::mat4& getProjectionMatrix() const = 0;
	virtual const glm::mat4& getViewMatrix() const = 0;
	virtual glm::mat4 getViewProjectionMatrix() const = 0;

	virtual float getAspectRatio() const = 0;
	virtual float getHorizontalFOVDegrees() const = 0;
	virtual float getVerticalFOVDegrees() const = 0;
	virtual float getZNear() const = 0;
	virtual float getZFar() const = 0;

	virtual glm::vec3 getCameraPositionFromViewMatrix() const = 0;
	virtual glm::vec3 getCameraForwardFromViewMatrix() const = 0;
	virtual glm::vec3 getCameraRightFromViewMatrix() const = 0;
	virtual glm::vec3 getCameraUpFromViewMatrix() const = 0;
	virtual glm::mat4 getCameraTransformFromViewMatrix() const = 0;
};
