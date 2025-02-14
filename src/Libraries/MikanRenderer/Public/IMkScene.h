#pragma once

#include "MkRendererFwd.h"

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

class IMkScene
{
public:
	virtual ~IMkScene() {}

	virtual void addInstance(IMkSceneRenderableConstPtr instance) = 0;
	virtual void removeInstance(IMkSceneRenderableConstPtr instance) = 0;
	virtual void removeAllInstances() = 0;

	virtual void setLightColor(const glm::vec4& lightColor) = 0;
	virtual const glm::vec4& getLightColor() const = 0;

	virtual void setLightDirection(const glm::vec3& lightDirection) = 0;
	virtual const glm::vec3& getLightDirection() const = 0;

	virtual void render(IMkCameraConstPtr camera, class MkStateStack& MkStateStack) const = 0;
};