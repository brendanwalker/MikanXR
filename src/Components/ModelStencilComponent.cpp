#include "AnchorObjectSystem.h"
#include "MikanAnchorComponent.h"
#include "MikanSceneComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "ModelStencilComponent.h"

#include <glm/gtx/matrix_decompose.hpp>

ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: MikanStencilComponent(owner)
	, ModelPosition(glm::vec3(0.f))
	, ModelQuat(glm::quat())
	, ModelScale(glm::vec3(1.f, 1.f, 1.f))
{}

void ModelStencilComponent::setModelStencil(const MikanStencilModel& stencil)
{
	StencilId = stencil.stencil_id;
	ParentAnchorId = stencil.parent_anchor_id;
	IsDisabled = stencil.is_disabled;
	StencilName = stencil.stencil_name;

	ModelPosition = MikanVector3f_to_glm_vec3(stencil.model_position);
	ModelQuat = MikanRotator3f_to_glm_quat(stencil.model_rotator);
	ModelScale = MikanVector3f_to_glm_vec3(stencil.model_scale);

	if (ComponentPropertyEvents::OnComponentChanged)
	{
		ComponentPropertyEvents::OnComponentChanged(*this);
	}

	updateSceneComponentTransform();
}

glm::mat4 ModelStencilComponent::getStencilLocalTransform() const
{
	return glm::translate(
		glm::mat4_cast(ModelQuat) * glm::scale(glm::mat4(1.f), ModelScale), 
		ModelPosition);
}

glm::mat4 ModelStencilComponent::getStencilWorldTransform() const
{
	const glm::mat4 localXform= getStencilLocalTransform();

	glm::mat4 worldXform = localXform;
	MikanAnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		worldXform = anchorPtr->getAnchorXform() * localXform;
	}

	return worldXform;
}

void ModelStencilComponent::setStencilLocalTransform(const glm::mat4& localXform)
{
	// Extract position scale and rotation from the local transform
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	if (glm::decompose(
		localXform,
		scale, orientation, translation, skew, perspective))
	{
		ModelPosition= translation;
		ModelQuat= orientation;
		ModelScale= scale;
	}

	notifyModelScaleChanged();
	notifyModelQuatChanged();
	notifyModelScaleChanged();

	updateSceneComponentTransform();
}

void ModelStencilComponent::setStencilWorldTransform(const glm::mat4& worldXform)
{
	glm::mat4 localXform = worldXform;
	MikanAnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		const glm::mat4 invParentXform = glm::inverse(anchorPtr->getAnchorXform());

		// Convert transform to the space of the parent anchor
		localXform = glm_composite_xform(worldXform, invParentXform);
	}

	setStencilLocalTransform(localXform);
}

void ModelStencilComponent::updateSceneComponentTransform()
{
	MikanSceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}