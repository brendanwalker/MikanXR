#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "BoxColliderComponent.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "SelectionComponent.h"
#include "TextStyle.h"

BoxStencilComponent::BoxStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
	, BoxCenter(glm::vec3(0.f))
	, BoxXAxis(glm::vec3(1.f, 0.f, 0.f))
	, BoxYAxis(glm::vec3(0.f, 1.f, 0.f))
	, BoxZAxis(glm::vec3(0.f, 0.f, 1.f))
	, BoxXSize(0.f)
	, BoxYSize(0.f)
	, BoxZSize(0.f)
{}

void BoxStencilComponent::init()
{
	MikanComponent::init();

	m_boxCollider = getOwnerObject()->getComponentOfType<BoxColliderComponent>();
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();
}

void BoxStencilComponent::update()
{
	MikanComponent::update();

	if (!IsDisabled)
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = m_sceneComponent.lock()->getWorldTransform();
		const glm::vec3 half_extents(BoxXSize / 2.f, BoxYSize / 2.f, BoxZSize / 2.f);
		const glm::vec3 position = glm::vec3(xform[3]);

		glm::vec3 color= Colors::DarkGray;
		SelectionComponentPtr selectionComponent= m_selectionComponent.lock();
		if (selectionComponent)
		{
			if (selectionComponent->getIsSelected())
				color= Colors::Yellow;
			else if (selectionComponent->getIsHovered())
				color= Colors::LightGray;
		}

		drawTransformedBox(xform, half_extents, color);
		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", StencilId);
	}
}

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
	updateBoxColliderExtents();
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
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		worldXform = anchorPtr->getAnchorXform() * localXform;
	}

	return worldXform;
}

void BoxStencilComponent::setStencilLocalTransformProperty(const glm::mat4& localXform)
{
	BoxXAxis = localXform[0];
	BoxYAxis = localXform[1];
	BoxZAxis = localXform[2];
	BoxCenter = localXform[3];

	notifyBoxXAxisChanged();
	notifyBoxYAxisChanged();
	notifyBoxZAxisChanged();
	notifyBoxCenterChanged();
}

void BoxStencilComponent::setStencilWorldTransformProperty(const glm::mat4& worldXform)
{
	glm::mat4 localXform = worldXform;
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(ParentAnchorId).lock();
	if (anchorPtr)
	{
		const glm::mat4 invParentXform = glm::inverse(anchorPtr->getAnchorXform());

		// Convert transform to the space of the parent anchor
		localXform = glm_composite_xform(worldXform, invParentXform);
	}

	setStencilLocalTransformProperty(localXform);
}

void BoxStencilComponent::updateSceneComponentTransform()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getStencilWorldTransform();

		sceneComponent->setWorldTransform(worldXform);
	}
}

void BoxStencilComponent::updateBoxColliderExtents()
{
	BoxColliderComponentPtr boxCollider= m_boxCollider.lock();
	if (boxCollider)
	{
		boxCollider->setHalfExtents(glm::vec3(BoxXSize, BoxYSize, BoxZSize) * 0.5f);
	}
}