#pragma once

#include "MikanComponent.h"
#include "MikanClientTypes.h"
#include "ComponentProperty.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class MikanSceneComponent;
using MikanSceneComponentWeakPtr= std::weak_ptr<MikanSceneComponent>;

class MikanStencilComponent;
typedef std::shared_ptr<MikanStencilComponent> MikanStencilComponentPtr;
typedef std::weak_ptr<MikanStencilComponent> MikanStencilComponentWeakPtr;

class MikanStencilComponent : public MikanComponent
{
public:
	MikanStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	MikanStencilID getStencilId() const { return StencilId; }
	MikanSpatialAnchorID getParentAnchorId() const { return ParentAnchorId; }
	bool getIsDisabled() const { return IsDisabled; }
	const std::string& getStencilName() const { return StencilName; }

	virtual glm::mat4 getStencilWorldTransform() const = 0;
	virtual void setStencilLocalTransform(const glm::mat4& xform) = 0;
	virtual void setStencilWorldTransform(const glm::mat4& xform) = 0;

protected:
	COMPONENT_PROPERTY(MikanStencilID, StencilId);
	COMPONENT_PROPERTY(MikanSpatialAnchorID, ParentAnchorId);
	COMPONENT_PROPERTY(bool, IsDisabled);
	COMPONENT_PROPERTY(std::string, StencilName);

	MikanSceneComponentWeakPtr m_sceneComponent;
};