#pragma once

#include "ComponentProperty.h"
#include "ComponentFwd.h"
#include "MikanClientTypes.h"
#include "RendererFwd.h"
#include "StencilComponent.h"

#include <memory>
#include <string>
#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class ModelStencilComponent : public StencilComponent
{
public:
	ModelStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;
	virtual void dispose() override;

	void setModelStencil(const MikanStencilModel& stencil);

	virtual glm::mat4 getStencilLocalTransform() const override;
	virtual glm::mat4 getStencilWorldTransform() const override;
	virtual void setStencilLocalTransformProperty(const glm::mat4& xform) override;
	virtual void setStencilWorldTransformProperty(const glm::mat4& xform) override;

	// Selection Events
	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionSelected();
	void onInteractionUnselected();

protected:
	COMPONENT_PROPERTY(glm::vec3, ModelPosition);
	COMPONENT_PROPERTY(glm::quat, ModelQuat);
	COMPONENT_PROPERTY(glm::vec3, ModelScale);

	void updateSceneComponentTransform();

	SelectionComponentWeakPtr m_selectionComponentWeakPtr;
	std::vector<GlStaticMeshInstancePtr> m_wireframeMeshes;
};
