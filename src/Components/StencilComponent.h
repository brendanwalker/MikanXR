#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "SceneFwd.h"

#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class StencilComponent : public MikanComponent
{
public:
	StencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;

	glm::mat4 getStencilLocalTransform() const;
	glm::mat4 getStencilWorldTransform() const;

	void attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId);

protected:
	virtual void onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr) {};

	SceneComponentWeakPtr m_sceneComponent;
};