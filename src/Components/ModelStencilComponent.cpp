#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "GlStaticMeshInstance.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "StaticMeshComponent.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "ModelStencilComponent.h"

#include <glm/gtx/matrix_decompose.hpp>

ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
	, ModelPosition(glm::vec3(0.f))
	, ModelQuat(glm::quat())
	, ModelScale(glm::vec3(1.f, 1.f, 1.f))
{}

void ModelStencilComponent::init()
{
	StencilComponent::init();

	// Get a list of all the attached wireframe meshes
	std::vector<StaticMeshComponentPtr> meshComponents;
	getOwnerObject()->getComponentsOfType<StaticMeshComponent>(meshComponents);
	for (auto& meshComponentPtr : meshComponents)
	{
		GlStaticMeshInstancePtr staticMeshPtr= meshComponentPtr->getStaticMesh();
		if (staticMeshPtr && staticMeshPtr->getName() == "wireframe")
		{
			m_wireframeMeshes.push_back(staticMeshPtr);
		}
	}
}

void ModelStencilComponent::update()
{
	StencilComponent::update();

	if (!IsDisabled)
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = m_sceneComponent.lock()->getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", StencilId);
	}
}

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
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
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
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
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
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}