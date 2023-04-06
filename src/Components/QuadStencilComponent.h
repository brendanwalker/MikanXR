#pragma once

#include "MikanStencilComponent.h"
#include "MikanClientTypes.h"
#include "ComponentProperty.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class QuadStencilComponent : public MikanStencilComponent
{
public:
	QuadStencilComponent(MikanObjectWeakPtr owner);

	void setQuadStencil(const MikanStencilQuad& stencil);

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
};
using MikanStencilComponentPtr = std::shared_ptr<MikanStencilComponent>;
using MikanStencilComponentWeakPtr = std::weak_ptr<MikanStencilComponent>;