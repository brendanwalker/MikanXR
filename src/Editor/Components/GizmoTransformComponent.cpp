#pragma once

#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "GizmoRotateComponent.h"
#include "GizmoScaleComponent.h"
#include "InputManager.h"
#include "MathGLM.h"
#include "MikanObject.h"
#include "SelectionComponent.h"

#include "SDL_keycode.h"

GizmoTransformComponent::GizmoTransformComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsCustomRender= true;
}

void GizmoTransformComponent::init()
{
	TransformComponent::init();

	MikanObjectPtr owner= getOwnerObject();

	GizmoTranslateComponentPtr translateComponent = owner->getComponentOfType<GizmoTranslateComponent>();
	GizmoRotateComponentPtr rotateComponent = owner->getComponentOfType<GizmoRotateComponent>();
	GizmoScaleComponentPtr scaleComponent = owner->getComponentOfType<GizmoScaleComponent>();

	translateComponent->OnTranslationRequested = MakeDelegate(this, &GizmoTransformComponent::onSelectionTranslationRequested);
	rotateComponent->OnRotateRequested = MakeDelegate(this, &GizmoTransformComponent::onSelectionRotationRequested);
	scaleComponent->OnScaleRequested = MakeDelegate(this, &GizmoTransformComponent::onSelectionScaleRequested);

	m_translateComponent = translateComponent;
	m_rotateComponent = rotateComponent;
	m_scaleComponent = scaleComponent;
}

void GizmoTransformComponent::bindInput()
{
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_t)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectTranslateMode);
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_r)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectRotateMode);
	InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_y)->OnKeyPressed +=
		MakeDelegate(this, &GizmoTransformComponent::selectScaleMode);
}

void GizmoTransformComponent::customRender()
{
	GizmoTranslateComponentPtr translatePtr = m_translateComponent.lock();
	GizmoRotateComponentPtr rotatePtr = m_rotateComponent.lock();
	GizmoScaleComponentPtr scalePtr = m_scaleComponent.lock();

	switch (m_gizmoMode)
	{
	case eGizmoMode::rotate:
		rotatePtr->customRender();
		break;
	case eGizmoMode::translate:
		translatePtr->customRender();
		break;
	case eGizmoMode::scale:
		scalePtr->customRender();
		break;
	}
}

void GizmoTransformComponent::setGizmoMode(eGizmoMode newMode)
{
	if (newMode == m_gizmoMode)
		return;

	GizmoTranslateComponentPtr translatePtr= m_translateComponent.lock();
	GizmoRotateComponentPtr rotatePtr= m_rotateComponent.lock();
	GizmoScaleComponentPtr scalePtr= m_scaleComponent.lock();

	translatePtr->setEnabled(false);
	rotatePtr->setEnabled(false);
	scalePtr->setEnabled(false);

	switch (newMode)
	{
	case eGizmoMode::rotate:
		rotatePtr->setEnabled(true);
		break;
	case eGizmoMode::translate:
		translatePtr->setEnabled(true);
		break;
	case eGizmoMode::scale:
		scalePtr->setEnabled(true);
		break;
	}

	m_gizmoMode= newMode;
}

void GizmoTransformComponent::selectTranslateMode()
{
	if (m_gizmoMode != eGizmoMode::none)
		setGizmoMode(eGizmoMode::translate);
}

void GizmoTransformComponent::selectRotateMode()
{
	if (m_gizmoMode != eGizmoMode::none)
		setGizmoMode(eGizmoMode::rotate);
}

void GizmoTransformComponent::selectScaleMode()
{
	if (m_gizmoMode != eGizmoMode::none)
		setGizmoMode(eGizmoMode::scale);
}

SelectionComponentPtr GizmoTransformComponent::getSelectionTarget() const
{
	return m_selectionTarget.lock();
}

void GizmoTransformComponent::onSelectionTranslationRequested(const glm::vec3& worldSpaceTranslation)
{
	// Translate the gizmo in world space
	glm::mat4 newGizmoWorldTransform = this->getWorldTransform();
	const glm::vec3 newGizmoPosition = glm_mat4_get_position(newGizmoWorldTransform) + worldSpaceTranslation;
	glm_mat4_set_position(newGizmoWorldTransform, newGizmoPosition);
	this->setWorldTransform(newGizmoWorldTransform);

	// Apply gizmo transform to gizmo's transform target
	applyTransformToTarget();
}

void GizmoTransformComponent::onSelectionRotationRequested(const glm::quat& worldSpaceRotation)
{
	// Rotate the gizmo in object space
	const glm::mat4 oldGizmoTransform = this->getWorldTransform();

	// Compute composite transform to apply world space rotation
	const glm::vec3 gizmoPosition = glm_mat4_get_position(oldGizmoTransform);
	const glm::mat4 undoTranslation = glm::translate(glm::mat4(1.f), -gizmoPosition);
	const glm::mat4 rotation = glm::mat4_cast(worldSpaceRotation);
	const glm::mat4 redoTranslation = glm::translate(glm::mat4(1.f), gizmoPosition);
	const glm::mat4 applyTransform =
		glm_composite_xform(glm_composite_xform(undoTranslation, rotation), redoTranslation);

	// Compute new gizmo worldspace transform 
	const glm::mat4 newGizmoTransform = glm_composite_xform(oldGizmoTransform, applyTransform);

	// Apply new gizmo transform to gizmo
	this->setWorldTransform(newGizmoTransform);

	// Apply gizmo transform to gizmo's transform target
	applyTransformToTarget();
}

void GizmoTransformComponent::onSelectionScaleRequested(const glm::vec3& objectSpaceScale)
{
	// Scale the gizmo in object space
	m_targetScale+= objectSpaceScale;

	// Apply gizmo transform to gizmo's transform target
	applyTransformToTarget();
}

void GizmoTransformComponent::setSelectionTarget(SelectionComponentPtr selectionTarget)
{
	assert(selectionTarget);

	eGizmoMode oldGizmoMode = getGizmoMode();
	eGizmoMode newGizmoMode = eGizmoMode::none;

	// Clean up old selection target state
	clearSelectionTarget();

	// Apply transforms to the root component of the mikan object that owns the selection target
	TransformComponentPtr transformTarget= selectionTarget->getOwnerObject()->getRootComponent();

	// Tell the new selection target that the gizmo is bound to it
	selectionTarget->notifyTransformGizmoBound();

	// Remember the new selection and transform target
	m_selectionTarget= selectionTarget;
	m_transformTarget= transformTarget;

	// Default to a transform mode gizmo if the gizmo wasn't active before
	if (oldGizmoMode != eGizmoMode::none)
		newGizmoMode = oldGizmoMode;
	else
		newGizmoMode = eGizmoMode::translate;

	// Update the desired gizmo state
	setGizmoMode(newGizmoMode);

	// Fetch target's transform and apply to gizmo
	applyTransformToGizmo();

	// Listen for scene component transform changes committed by the UI
	transformTarget->getDefinition()->OnMarkedDirty+= 
		MakeDelegate(this, &GizmoTransformComponent::onTransformTargetConfigChange);
}

void GizmoTransformComponent::clearSelectionTarget()
{
	TransformComponentPtr oldTransformTarget= m_transformTarget.lock();
	SelectionComponentPtr oldSelectionTarget= m_selectionTarget.lock();

	// Tell the old selection target that the gizmo is no longer bound to it
	if (oldSelectionTarget)
	{
		oldSelectionTarget->notifyTransformGizmoUnbound();
	}

	// Stop listen for scene component transform changes committed by the UI
	if (oldTransformTarget)
	{
		oldTransformTarget->getDefinition()->OnMarkedDirty -=
			MakeDelegate(this, &GizmoTransformComponent::onTransformTargetConfigChange);
	}

	m_transformTarget.reset();
	m_selectionTarget.reset();
	setGizmoMode(eGizmoMode::none);
	setWorldTransform(glm::mat4(1.f));
}

void GizmoTransformComponent::applyTransformToGizmo()
{
	assert(!m_bIsApplyingTransformToTarget);

	TransformComponentPtr transformComponentTarget = m_transformTarget.lock();

	if (transformComponentTarget)
	{
		// Extract scale from rotation&translation on target world transform
		const glm::mat4 srtTransform = transformComponentTarget->getWorldTransform();
		const glm::vec3 xAxis = glm_mat4_get_x_axis(srtTransform);
		const glm::vec3 yAxis = glm_mat4_get_y_axis(srtTransform);
		const glm::vec3 zAxis = glm_mat4_get_z_axis(srtTransform);
		const glm::vec3 position = glm_mat4_get_position(srtTransform);
		const float xScale = glm::length(xAxis);
		const float yScale = glm::length(yAxis);
		const float zScale = glm::length(zAxis);
		const glm::mat4 rtTransform = glm::mat4(
			glm::vec4(xAxis / xScale, 0.f),
			glm::vec4(yAxis / yScale, 0.f),
			glm::vec4(zAxis / zScale, 0.f),
			glm::vec4(position, 1.f));

		// Snap the gizmo to the target scene component
		setWorldTransform(rtTransform);

		// Store the target scene component's scale outside of the gizmo transform
		m_targetScale = glm::vec3(xScale, yScale, zScale);
	}
}


void GizmoTransformComponent::applyTransformToTarget()
{
	TransformComponentPtr transformComponentTarget= m_transformTarget.lock();

	if (transformComponentTarget)
	{
		// Apply scale back to rotation&translation on target world transform
		const glm::mat4 rtTransform = getWorldTransform();
		const glm::vec3 xAxis = glm_mat4_get_x_axis(rtTransform);
		const glm::vec3 yAxis = glm_mat4_get_y_axis(rtTransform);
		const glm::vec3 zAxis = glm_mat4_get_z_axis(rtTransform);
		const glm::vec3 position = glm_mat4_get_position(rtTransform);
		const glm::mat4 srtTransform = glm::mat4(
			glm::vec4(xAxis * m_targetScale.x, 0.f),
			glm::vec4(yAxis * m_targetScale.y, 0.f),
			glm::vec4(zAxis * m_targetScale.z, 0.f),
			glm::vec4(position, 1.f));

		m_bIsApplyingTransformToTarget = true;
		transformComponentTarget->setWorldTransform(srtTransform);
		m_bIsApplyingTransformToTarget = false;
	}
}

void GizmoTransformComponent::onTransformTargetConfigChange(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	// Did a transform property of the gizmo target change?
	if (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativePositionPropertyId) ||
		changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeRotationPropertyId) ||
		changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeScalePropertyId))
	{
		// Ignore if this gizmo was the one applying the change (in applyTransformToTarget)
		// Otherwise this was a change committed by the UI
		if (!m_bIsApplyingTransformToTarget)
		{
			applyTransformToGizmo();
		}
	}
}