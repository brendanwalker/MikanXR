#pragma once

#include "MikanStencilComponent.h"
#include "MikanClientTypes.h"
#include "ComponentProperty.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class ModelStencilComponent : public MikanStencilComponent
{
public:
	ModelStencilComponent(MikanObjectWeakPtr owner);

	void setModelStencil(const MikanStencilModel& stencil);

	virtual glm::mat4 getStencilLocalTransform() const override;
	virtual glm::mat4 getStencilWorldTransform() const override;
	virtual void setStencilLocalTransform(const glm::mat4& xform) override;
	virtual void setStencilWorldTransform(const glm::mat4& xform) override;

protected:
	COMPONENT_PROPERTY(glm::vec3, ModelPosition);
	COMPONENT_PROPERTY(glm::quat, ModelQuat);
	COMPONENT_PROPERTY(glm::vec3, ModelScale);

	void updateSceneComponentTransform();
};
using MikanStencilComponentPtr = std::shared_ptr<MikanStencilComponent>;
using MikanStencilComponentWeakPtr = std::weak_ptr<MikanStencilComponent>;