#include "AnchorObjectSystem.h"
#include "MikanAnchorComponent.h"
#include "MikanSceneComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "BoxStencilComponent.h"

BoxStencilComponent::BoxStencilComponent(MikanObjectWeakPtr owner)
	: MikanStencilComponent(owner)
	, BoxCenter(glm::vec3(0.f))
	, BoxXAxis(glm::vec3(1.f, 0.f, 0.f))
	, BoxYAxis(glm::vec3(0.f, 1.f, 0.f))
	, BoxZAxis(glm::vec3(0.f, 0.f, 1.f))
	, BoxXSize(0.f)
	, BoxYSize(0.f)
	, BoxZSize(0.f)
{}

void BoxStencilComponent::setBoxStencil(const MikanStencilBox& stencil)
{
	StencilId = stencil.stencil_id;
	ParentAnchorId = stencil.parent_anchor_id;
	BoxCenter = MikanVector3f_to_glm_vec3(stencil.box_center);
	BoxXAxis = MikanVector3f_to_glm_vec3(stencil.box_x_axis);
	BoxYAxis = MikanVector3f_to_glm_vec3(stencil.box_y_axis);
	BoxZAxis = MikanVector3f_to_glm_vec3(stencil.box_z_axis);
	BoxXSize = stencil.box_x_size;
	BoxYSize = stencil.box_y_size;
	BoxZSize = stencil.box_z_size;
	IsDisabled = stencil.is_disabled;
	StencilName = stencil.stencil_name;

	if (ComponentPropertyEvents::OnComponentChanged)
	{
		ComponentPropertyEvents::OnComponentChanged(*this);
	}

	updateSceneComponentTransform();
}

glm::mat4 BoxStencilComponent::getStencilLocalTransform() const
{
	return glm::mat4(
		glm::vec4(BoxXAxis, 0.f),
		glm::vec4(BoxYAxis, 0.f),
		glm::vec4(BoxZAxis, 0.f),
		glm::vec4(BoxCenter, 1.f));
}

glm::mat4 BoxStencilComponent::getStencilWorldTransform() const
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

void BoxStencilComponent::setStencilLocalTransform(const glm::mat4& localXform)
{
	BoxXAxis = localXform[0];
	BoxYAxis = localXform[1];
	BoxZAxis = localXform[2];
	BoxCenter = localXform[3];

	notifyBoxXAxisChanged();
	notifyBoxYAxisChanged();
	notifyBoxZAxisChanged();
	notifyBoxCenterChanged();

	updateSceneComponentTransform();
}

void BoxStencilComponent::setStencilWorldTransform(const glm::mat4& worldXform)
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

void BoxStencilComponent::updateSceneComponentTransform()
{
	MikanSceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}