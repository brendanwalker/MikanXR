#pragma once

#include "MikanStencilComponent.h"
#include "MikanClientTypes.h"
#include "ComponentProperty.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"

class MikanObject;
using MikanObjectWeakPtr = std::weak_ptr<MikanObject>;

class MikanBoxColliderComponent;
using MikanBoxColliderComponentWeakPtr = std::weak_ptr<MikanBoxColliderComponent>;

class BoxStencilComponent : public MikanStencilComponent
{
public:
	BoxStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	void setBoxStencil(const MikanStencilBox& stencil);

	virtual glm::mat4 getStencilLocalTransform() const override;
	virtual glm::mat4 getStencilWorldTransform() const override;
	virtual void setStencilLocalTransform(const glm::mat4& xform) override;
	virtual void setStencilWorldTransform(const glm::mat4& xform) override;

protected:
	COMPONENT_PROPERTY(glm::vec3, BoxCenter);
	COMPONENT_PROPERTY(glm::vec3, BoxXAxis);
	COMPONENT_PROPERTY(glm::vec3, BoxYAxis);
	COMPONENT_PROPERTY(glm::vec3, BoxZAxis);
	COMPONENT_PROPERTY(float, BoxXSize);
	COMPONENT_PROPERTY(float, BoxYSize);
	COMPONENT_PROPERTY(float, BoxZSize);

	void updateSceneComponentTransform();
	void updateBoxColliderExtents();

	MikanBoxColliderComponentWeakPtr m_boxCollider;
};
using MikanStencilComponentPtr = std::shared_ptr<MikanStencilComponent>;
using MikanStencilComponentWeakPtr = std::weak_ptr<MikanStencilComponent>;