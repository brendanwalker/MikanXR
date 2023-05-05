#include "AnchorObjectSystem.h"
#include "App.h"
#include "BoxColliderComponent.h"
#include "DiskColliderComponent.h"
#include "EditorObjectSystem.h"
#include "GizmoRotateComponent.h"
#include "GizmoScaleComponent.h"
#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "GlViewport.h"
#include "ObjectSystemManager.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanScene.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"

void EditorObjectSystem::init()
{
	MikanObjectSystem::init();

	m_lastestRaycastResult= ColliderRaycastHitResult();
	m_hoverComponentWeakPtr.reset();
	m_selectedComponentWeakPtr.reset();

	m_scene = std::make_shared<MikanScene>();
	m_scene->init();

	ObjectSystemManagerPtr objSystemMgr= App::getInstance()->getObjectSystemManager();

	AnchorObjectSystemPtr anchorObjectSystem= objSystemMgr->getSystemOfType<AnchorObjectSystem>();
	anchorObjectSystem->OnObjectAdded+= MakeDelegate(this, &EditorObjectSystem::onObjectAdded);
	anchorObjectSystem->OnObjectRemoved+= MakeDelegate(this, &EditorObjectSystem::onObjectRemoved);
	for (MikanObjectPtr objectPtr : anchorObjectSystem->getObjectList())
	{
		m_scene->addMikanObject(objectPtr);
	}

	StencilObjectSystemPtr stencilObjectSystem= objSystemMgr->getSystemOfType<StencilObjectSystem>();
	stencilObjectSystem->OnObjectAdded += MakeDelegate(this, &EditorObjectSystem::onObjectAdded);
	stencilObjectSystem->OnObjectRemoved += MakeDelegate(this, &EditorObjectSystem::onObjectRemoved);
	for (MikanObjectPtr objectPtr : stencilObjectSystem->getObjectList())
	{
		m_scene->addMikanObject(objectPtr);
	}

	createTransformGizmo();
}

void EditorObjectSystem::createTransformGizmo()
{
	m_gizmoObjectWeakPtr = newObject();
	MikanObjectPtr gizmoObjectPtr= m_gizmoObjectWeakPtr.lock();
	gizmoObjectPtr->setName("Gizmo");

	GizmoTransformComponentPtr transformGizmoPtr= gizmoObjectPtr->addComponent<GizmoTransformComponent>();
	gizmoObjectPtr->setRootComponent(transformGizmoPtr);
	m_gizmoComponentWeakPtr= transformGizmoPtr;

	gizmoObjectPtr->addComponent<SelectionComponent>();

	const float W= 0.01f;
	const float R= 0.5f;

	GizmoTranslateComponentPtr translateComponentPtr= gizmoObjectPtr->addComponent<GizmoTranslateComponent>();	
	createGizmoBoxCollider(gizmoObjectPtr, "centerTranslateHandle", glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.01f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "xyTranslateHandle", glm::vec3(0.025f, 0.025f, 0.f), glm::vec3(0.025f, 0.025f, 0.001f));
	createGizmoBoxCollider(gizmoObjectPtr, "xzTranslateHandle", glm::vec3(0.025f, 0.f, 0.025f), glm::vec3(0.025f, 0.001f, 0.025f));
	createGizmoBoxCollider(gizmoObjectPtr, "yzTranslateHandle", glm::vec3(0.f, 0.025f, 0.025f), glm::vec3(0.001f, 0.025f, 0.025f));
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisTranslateHandle", glm::vec3(R/2.f, 0.f, 0.f), glm::vec3(R/2.f, W, W));
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisTranslateHandle", glm::vec3(0.f, R/2.f, 0.f), glm::vec3(W, R/2.f, W));
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisTranslateHandle", glm::vec3(0.f, 0.f, R/2.f), glm::vec3(W, W, R/2.f));
	translateComponentPtr->OnTranslationRequested= MakeDelegate(this, &EditorObjectSystem::onSelectionTranslationRequested);

	GizmoRotateComponentPtr rotateComponentPtr= gizmoObjectPtr->addComponent<GizmoRotateComponent>();
	createGizmoDiskCollider(gizmoObjectPtr, "xAxisRotateHandle", glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), R);
	createGizmoDiskCollider(gizmoObjectPtr, "yAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), R);
	createGizmoDiskCollider(gizmoObjectPtr, "zAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), R);
	rotateComponentPtr->OnRotateRequested= MakeDelegate(this, &EditorObjectSystem::onSelectionRotationRequested);

	GizmoScaleComponentPtr scaleComponentPtr= gizmoObjectPtr->addComponent<GizmoScaleComponent>();
	createGizmoBoxCollider(gizmoObjectPtr, "centerScaleHandle", glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.01f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisScaleHandle", glm::vec3(R/2.f, 0.f, 0.f), glm::vec3(R/2.f, W, W));
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisScaleHandle", glm::vec3(0.f, R/2.f, 0.f), glm::vec3(W, R/2.f, W));
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisScaleHandle", glm::vec3(0.f, 0.f, R/2.f), glm::vec3(W, W, R/2.f));
	scaleComponentPtr->OnScaleRequested= MakeDelegate(this, &EditorObjectSystem::onSelectionScaleRequested);

	gizmoObjectPtr->init();

	m_scene->addMikanObject(m_gizmoObjectWeakPtr);
}

void EditorObjectSystem::createGizmoBoxCollider(
	MikanObjectPtr gizmoObjectPtr,
	const std::string& name,
	const glm::vec3& center,
	const glm::vec3& halfExtents)
{
	BoxColliderComponentPtr colliderPtr= gizmoObjectPtr->addComponent<BoxColliderComponent>(name);

	colliderPtr->setName(name);
	colliderPtr->setHalfExtents(halfExtents);
	colliderPtr->setRelativeTransform(GlmTransform(center));
	colliderPtr->attachToComponent(gizmoObjectPtr->getRootComponent());
	colliderPtr->setEnabled(false);
}

void EditorObjectSystem::createGizmoDiskCollider(
	MikanObjectPtr gizmoObjectPtr,
	const std::string& name,
	const glm::vec3& center,
	const glm::vec3& normal,
	const float radius)
{
	DiskColliderComponentPtr colliderPtr = gizmoObjectPtr->addComponent<DiskColliderComponent>(name);

	glm::quat orientation= glm::quat(glm::vec3(0.f, 1.f, 0.f), normal);
	colliderPtr->setRelativeTransform(GlmTransform(center, orientation));
	colliderPtr->attachToComponent(gizmoObjectPtr->getRootComponent());
	colliderPtr->setRadius(radius);
	colliderPtr->setName(name);
	colliderPtr->setEnabled(false);
}

void EditorObjectSystem::dispose()
{
	m_gizmoObjectWeakPtr.reset();
	m_gizmoComponentWeakPtr.reset();
	m_scene= nullptr;

	MikanObjectSystem::dispose();
}

void EditorObjectSystem::bindViewport(GlViewportWeakPtr viewportWeakPtr)
{
	GlViewportPtr viewportPtr= viewportWeakPtr.lock();
	if (!viewportPtr)
		return;

	viewportPtr->OnMouseRayChanged+= MakeDelegate(this, &EditorObjectSystem::onMouseRayChanged);
	viewportPtr->OnMouseRayButtonUp+= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonUp);
	viewportPtr->OnMouseRayButtonDown+= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonDown);

	m_viewports.push_back(viewportWeakPtr);
}

void EditorObjectSystem::clearViewports()
{
	for (GlViewportWeakPtr& viewportWeakPtr : m_viewports)
	{
		GlViewportPtr viewportPtr = viewportWeakPtr.lock();
		if (viewportPtr)
		{
			viewportPtr->OnMouseRayChanged -= MakeDelegate(this, &EditorObjectSystem::onMouseRayChanged);
			viewportPtr->OnMouseRayButtonUp -= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonUp);
			viewportPtr->OnMouseRayButtonDown -= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonDown);
		}
	}
	m_viewports.clear();
}

void EditorObjectSystem::onObjectAdded(MikanObjectSystem& system, MikanObject& object)
{
	m_scene->addMikanObject(object.shared_from_this());
}

void EditorObjectSystem::onObjectRemoved(MikanObjectSystem& system, MikanObject& object)
{
	m_scene->removeMikanObject(object.shared_from_this());
}

void EditorObjectSystem::onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr currentHoverPtr= m_hoverComponentWeakPtr.lock();

	if (button == SDL_BUTTON_LEFT)
	{
		// See if the current selection is changing
		SelectionComponentPtr oldSelectedComponentPtr = m_selectedComponentWeakPtr.lock();
		SelectionComponentPtr newSelectedComponentPtr = currentHoverPtr;
		if (oldSelectedComponentPtr != newSelectedComponentPtr)
		{
			// Update the selection component weak ptr
			m_selectedComponentWeakPtr = newSelectedComponentPtr;

			// Send notification of selection change
			onSelectionChanged(oldSelectedComponentPtr, newSelectedComponentPtr);
		}

		// Send notification of selection grab
		if (newSelectedComponentPtr)
		{
			newSelectedComponentPtr->notifyGrab(m_lastestRaycastResult);
		}
	}
}

void EditorObjectSystem::onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderRaycastHitResult prevRaycastResult = m_lastestRaycastResult;
	m_lastestRaycastResult = ColliderRaycastHitResult();

	SelectionComponentPtr oldHoverComponentPtr = m_hoverComponentWeakPtr.lock();
	SelectionComponentPtr newHoverComponentPtr =
		findClosestSelectionTarget(rayOrigin, rayDir, m_lastestRaycastResult);

	if (newHoverComponentPtr && !oldHoverComponentPtr)
	{
		m_hoverComponentWeakPtr = newHoverComponentPtr;
		newHoverComponentPtr->notifyHoverEnter(m_lastestRaycastResult);
	}
	else if (!newHoverComponentPtr && oldHoverComponentPtr)
	{
		oldHoverComponentPtr->notifyHoverExit(prevRaycastResult);
		m_hoverComponentWeakPtr = SelectionComponentWeakPtr();
	}

	SelectionComponentPtr selectedComponentPtr = m_selectedComponentWeakPtr.lock();
	if (selectedComponentPtr)
	{
		selectedComponentPtr->notifyMove(rayOrigin, rayDir);
	}
}

void EditorObjectSystem::onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr currentSelectedPtr = m_selectedComponentWeakPtr.lock();
	if (currentSelectedPtr)
	{
		currentSelectedPtr->notifyRelease();
	}
}

void EditorObjectSystem::onSelectionChanged(
	SelectionComponentPtr oldSelectedComponentPtr, 
	SelectionComponentPtr newSelectedComponentPtr)
{
	GizmoTransformComponentPtr gizmoComponentPtr= m_gizmoComponentWeakPtr.lock();
	eGizmoMode oldGizmoMode= gizmoComponentPtr->getGizmoMode();
	eGizmoMode newGizmoMode= oldGizmoMode;

	// Tell the old selection that it's getting unselected
	if (oldSelectedComponentPtr)
	{
		oldSelectedComponentPtr->notifyUnselected();

		newGizmoMode= eGizmoMode::none;
	}

	// Tell the new selection that it's getting selected
	if (newSelectedComponentPtr)
	{
		newSelectedComponentPtr->notifySelected();

		if (oldGizmoMode != eGizmoMode::none)
			newGizmoMode= oldGizmoMode;
		else
			newGizmoMode= eGizmoMode::translate;

		// Snap gizmo to the newly selected component
		SceneComponentPtr selectedRootPtr= newSelectedComponentPtr->getOwnerObject()->getRootComponent();
		if (selectedRootPtr)
		{
			gizmoComponentPtr->setWorldTransform(selectedRootPtr->getWorldTransform());
		}
	}

	// Update the desired gizmo state
	gizmoComponentPtr->setGizmoMode(newGizmoMode);
}

void EditorObjectSystem::onSelectionTranslationRequested(const glm::vec3& worldSpaceTranslation)
{
	// Translate the gizmo in world space
	GizmoTransformComponentPtr gizmoComponentPtr= m_gizmoComponentWeakPtr.lock();
	const glm::mat4 oldGizmoXform= gizmoComponentPtr->getWorldTransform();
	const glm::mat4 newGizmoXform= glm::translate(oldGizmoXform, worldSpaceTranslation);
	gizmoComponentPtr->setWorldTransform(newGizmoXform);

	// If we have a selected object, snap it to the gizmo transform
	SelectionComponentPtr selectedComponentPtr= m_selectedComponentWeakPtr.lock();
	if (selectedComponentPtr)
	{
		SceneComponentPtr selectedRootWeakPtr= selectedComponentPtr->getOwnerObject()->getRootComponent();
		SceneComponentPtr selectedRootPtr= selectedRootWeakPtr;
		if (selectedRootPtr)
		{
			selectedRootPtr->setWorldTransform(newGizmoXform);
		}
	}
}

void EditorObjectSystem::onSelectionRotationRequested(const glm::quat& objectSpaceRotation)
{
	// Scale the gizmo in object space
	GizmoTransformComponentPtr gizmoComponentPtr = m_gizmoComponentWeakPtr.lock();
	GlmTransform relativeTransform = gizmoComponentPtr->getRelativeTransform();
	relativeTransform.setOrientation(objectSpaceRotation);
	gizmoComponentPtr->setRelativeTransform(relativeTransform);

	// If we have a selected object, snap it to the gizmo transform
	SelectionComponentPtr selectedComponentPtr = m_selectedComponentWeakPtr.lock();
	if (selectedComponentPtr)
	{
		SceneComponentPtr selectedRootWeakPtr = selectedComponentPtr->getOwnerObject()->getRootComponent();
		SceneComponentPtr selectedRootPtr = selectedRootWeakPtr;
		if (selectedRootPtr)
		{
			selectedRootPtr->setRelativeTransform(relativeTransform);
		}
	}
}

void EditorObjectSystem::onSelectionScaleRequested(const glm::vec3& objectSpaceScale)
{
	// Scale the gizmo in object space
	GizmoTransformComponentPtr gizmoComponentPtr = m_gizmoComponentWeakPtr.lock();
	GlmTransform relativeTransform = gizmoComponentPtr->getRelativeTransform();
	relativeTransform.setScale(objectSpaceScale);
	gizmoComponentPtr->setRelativeTransform(relativeTransform);

	// If we have a selected object, snap it to the gizmo transform
	SelectionComponentPtr selectedComponentPtr = m_selectedComponentWeakPtr.lock();
	if (selectedComponentPtr)
	{
		SceneComponentPtr selectedRootWeakPtr = selectedComponentPtr->getOwnerObject()->getRootComponent();
		SceneComponentPtr selectedRootPtr = selectedRootWeakPtr;
		if (selectedRootPtr)
		{
			selectedRootPtr->setRelativeTransform(relativeTransform);
		}
	}
}

SelectionComponentPtr EditorObjectSystem::findClosestSelectionTarget(
	const glm::vec3& rayOrigin, 
	const glm::vec3& rayDir,
	ColliderRaycastHitResult& outRaycastResult) const
{
	SelectionComponentPtr closestSelectionComponent;
	
	outRaycastResult.hitDistance= k_real_max;
	outRaycastResult.hitLocation = glm::vec3();
	outRaycastResult.hitNormal = glm::vec3();

	for (SelectionComponentWeakPtr selectionComponentWeakPtr : m_scene->getSelectionComponentList())
	{
		SelectionComponentPtr selectionComponentPtr= selectionComponentWeakPtr.lock();
		if (!selectionComponentPtr)
			continue;

		ColliderRaycastHitRequest request= { rayOrigin, rayDir };
		ColliderRaycastHitResult result;

		if (selectionComponentPtr->computeRayIntersection(request, result) && 
			result.hitDistance < outRaycastResult.hitDistance)
		{
			closestSelectionComponent= selectionComponentPtr;
			outRaycastResult= result;
		}
	}

	return closestSelectionComponent;
}