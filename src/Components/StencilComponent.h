#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "ComponentProperty.h"

#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class StencilComponent : public MikanComponent
{
public:
	StencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	MikanStencilID getStencilId() const { return StencilId; }
	MikanSpatialAnchorID getParentAnchorId() const { return ParentAnchorId; }
	bool getIsDisabled() const { return IsDisabled; }
	const std::string& getStencilName() const { return StencilName; }

	virtual glm::mat4 getStencilLocalTransform() const = 0;
	virtual glm::mat4 getStencilWorldTransform() const = 0;
	virtual void setStencilLocalTransform(const glm::mat4& xform) = 0;
	virtual void setStencilWorldTransform(const glm::mat4& xform) = 0;

protected:
	COMPONENT_PROPERTY(MikanStencilID, StencilId);
	COMPONENT_PROPERTY(MikanSpatialAnchorID, ParentAnchorId);
	COMPONENT_PROPERTY(bool, IsDisabled);
	COMPONENT_PROPERTY(std::string, StencilName);

	SceneComponentWeakPtr m_sceneComponent;
};