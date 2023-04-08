#include "AnchorObjectSystem.h"
#include "MikanAnchorComponent.h"
#include "MikanSceneComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "QuadStencilComponent.h"

QuadStencilComponent::QuadStencilComponent(MikanObjectWeakPtr owner)
	: MikanStencilComponent(owner)
	, QuadCenter(glm::vec3(0.f))
	, QuadXAxis(glm::vec3(1.f, 0.f, 0.f))
	, QuadYAxis(glm::vec3(0.f, 1.f, 0.f))
	, QuadNormal(glm::vec3(0.f, 0.f, 1.f))
	, QuadWidth(0.f)
	, QuadHeight(0.f)
	, IsDoubleSided(false)
{
}

void QuadStencilComponent::setQuadStencil(const MikanStencilQuad& stencil)
{
	StencilId= stencil.stencil_id;
	ParentAnchorId= stencil.parent_anchor_id;
	QuadCenter= MikanVector3f_to_glm_vec3(stencil.quad_center);
	QuadXAxis= MikanVector3f_to_glm_vec3(stencil.quad_x_axis);
	QuadYAxis= MikanVector3f_to_glm_vec3(stencil.quad_y_axis);
	QuadNormal= MikanVector3f_to_glm_vec3(stencil.quad_normal);
	QuadWidth= stencil.quad_width;
	QuadHeight= stencil.quad_height;
	IsDoubleSided= stencil.is_double_sided;
	IsDisabled= stencil.is_disabled;
	StencilName= stencil.stencil_name;

	if (ComponentPropertyEvents::OnComponentChanged)
	{
		ComponentPropertyEvents::OnComponentChanged(*this);
	}

	updateSceneComponentTransform();
}

glm::mat4 QuadStencilComponent::getStencilLocalTransform() const
{
	return glm::mat4(
		glm::vec4(QuadXAxis, 0.f),
		glm::vec4(QuadYAxis, 0.f),
		glm::vec4(QuadNormal, 0.f),
		glm::vec4(QuadCenter, 1.f));
}

glm::mat4 QuadStencilComponent::getStencilWorldTransform() const
{
	const glm::mat4 localXform= getStencilLocalTransform();

	glm::mat4 worldXform= localXform;
	MikanAnchorComponentPtr anchorPtr= AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		worldXform = anchorPtr->getAnchorXform() * localXform;
	}

	return worldXform;
}

void QuadStencilComponent::setStencilLocalTransform(const glm::mat4& localXform)
{
	QuadXAxis = localXform[0];
	QuadYAxis = localXform[1];
	QuadNormal = localXform[2];
	QuadCenter = localXform[3];

	notifyQuadXAxisChanged();
	notifyQuadYAxisChanged();
	notifyQuadNormalChanged();
	notifyQuadCenterChanged();

	updateSceneComponentTransform();
}

void QuadStencilComponent::setStencilWorldTransform(const glm::mat4& worldXform)
{
	glm::mat4 localXform = worldXform;
	MikanAnchorComponentPtr anchorPtr= AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		const glm::mat4 invParentXform = glm::inverse(anchorPtr->getAnchorXform());

		// Convert transform to the space of the parent anchor
		localXform = glm_composite_xform(worldXform, invParentXform);
	}

	setStencilLocalTransform(localXform);
}

void QuadStencilComponent::updateSceneComponentTransform()
{
	MikanSceneComponentPtr sceneComponent= m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}