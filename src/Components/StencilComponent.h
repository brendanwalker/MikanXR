#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "SceneFwd.h"
#include "Transform.h"

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

	virtual void setConfigTransform(const GlmTransform& transform) = 0;
	virtual const GlmTransform getConfigTransform() = 0;

	virtual MikanStencilID getParentAnchorId() const = 0;

	void attachSceneComponentToAnchor(MikanSpatialAnchorID newParentId);

protected:
	void onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr);
	void applyConfigTransformToSceneComponent();

	bool m_bUpdatingSceneComponentTransform = false;
	SceneComponentWeakPtr m_sceneComponent;
};