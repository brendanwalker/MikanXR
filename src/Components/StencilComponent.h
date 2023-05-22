#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "SceneComponent.h"

#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class StencilComponent : public SceneComponent
{
public:
	StencilComponent(MikanObjectWeakPtr owner);

	virtual MikanStencilID getParentAnchorId() const = 0;

	void attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId);
};