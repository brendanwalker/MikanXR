#pragma once

#include "ComponentProperty.h"
#include "MikanClientTypes.h"
#include "StencilComponent.h"

#include <string>

#include "glm/ext/vector_float3.hpp"

class QuadStencilComponent : public StencilComponent
{
public:
	QuadStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;

	void setQuadStencil(const MikanStencilQuad& stencil);

	virtual glm::mat4 getStencilLocalTransform() const override;
	virtual glm::mat4 getStencilWorldTransform() const override;
	virtual void setStencilLocalTransform(const glm::mat4& xform) override;
	virtual void setStencilWorldTransform(const glm::mat4& xform) override;

protected:
	COMPONENT_PROPERTY(glm::vec3, QuadCenter);
	COMPONENT_PROPERTY(glm::vec3, QuadXAxis);
	COMPONENT_PROPERTY(glm::vec3, QuadYAxis);
	COMPONENT_PROPERTY(glm::vec3, QuadNormal);
	COMPONENT_PROPERTY(float, QuadWidth);
	COMPONENT_PROPERTY(float, QuadHeight);
	COMPONENT_PROPERTY(bool, IsDoubleSided);

	void updateSceneComponentTransform();
	void updateBoxColliderExtents();

	BoxColliderComponentWeakPtr m_boxCollider;
};