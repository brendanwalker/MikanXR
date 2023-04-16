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

EditorObjectSystem::EditorObjectSystem() : MikanObjectSystem()
{
}

EditorObjectSystem::~EditorObjectSystem()
{
	dispose();
}

void EditorObjectSystem::init()
{
	MikanObjectSystem::init();

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

	GizmoTransformComponentPtr transformGizmoPtr= gizmoObjectPtr->addComponent<GizmoTransformComponent>();
	gizmoObjectPtr->setRootComponent(transformGizmoPtr);

	const float R= 0.5f;

	gizmoObjectPtr->addComponent<GizmoTranslateComponent>();	
	createGizmoBoxCollider(gizmoObjectPtr, "centerTranslateHandle", glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.01f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "xyTranslateHandle", glm::vec3(0.025f, 0.025f, 0.f), glm::vec3(0.025f, 0.025f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "xzTranslateHandle", glm::vec3(0.025f, 0.f, 0.025f), glm::vec3(0.025f, 0.01f, 0.025f));
	createGizmoBoxCollider(gizmoObjectPtr, "yzTranslateHandle", glm::vec3(0.f, 0.025f, 0.025f), glm::vec3(0.01f, 0.025f, 0.025f));
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisTranslateHandle", glm::vec3(R/2.f, 0.f, 0.f), glm::vec3(R/2.f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisTranslateHandle", glm::vec3(0.f, R/2.f, 0.f), glm::vec3(0.01f, R/2.f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisTranslateHandle", glm::vec3(0.f, 0.f, R/2.f), glm::vec3(0.01f, 0.01f, R/2.f));

	gizmoObjectPtr->addComponent<GizmoRotateComponent>();
	createGizmoDiskCollider(gizmoObjectPtr, "xAxisRotateHandle", glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), R);
	createGizmoDiskCollider(gizmoObjectPtr, "yAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), R);
	createGizmoDiskCollider(gizmoObjectPtr, "zAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), R);

	gizmoObjectPtr->addComponent<GizmoScaleComponent>();
	createGizmoBoxCollider(gizmoObjectPtr, "centerScaleHandle", glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.01f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisScaleHandle", glm::vec3(R/2.f, 0.f, 0.f), glm::vec3(R/2.f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisScaleHandle", glm::vec3(0.f, R/2.f, 0.f), glm::vec3(0.01f, R/2.f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisScaleHandle", glm::vec3(0.f, 0.f, R/2.f), glm::vec3(0.01f, 0.01f, R/2.f));
}

void EditorObjectSystem::createGizmoBoxCollider(
	MikanObjectPtr gizmoObjectPtr,
	const std::string& name,
	const glm::vec3& center,
	const glm::vec3& halfExtents)
{
	BoxColliderComponentPtr colliderPtr= gizmoObjectPtr->addComponent<BoxColliderComponent>(name);

	colliderPtr->setHalfExtents(halfExtents);
	colliderPtr->setRelativeTransform(GlmTransform(center));
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
	colliderPtr->setRadius(radius);
}

void EditorObjectSystem::dispose()
{
	m_gizmoObjectWeakPtr.reset();
	m_gizmoComponentWeakPtr.reset();
	m_scene= nullptr;
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

void EditorObjectSystem::onMouseRayChanged(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
	ColliderRaycastHitResult raycastResult;
	SelectionComponentWeakPtr newHoverComponentWeakPtr= findClosestSelectionTarget(rayOrigin, rayDir, raycastResult);
	SelectionComponentPtr newHoverComponentPtr= newHoverComponentWeakPtr.lock();
	SelectionComponentPtr selectionHoverPtr= m_hoverComponentWeakPtr.lock();

	if (newHoverComponentPtr && !selectionHoverPtr)
	{
		m_hoverComponentWeakPtr= newHoverComponentWeakPtr;
		newHoverComponentPtr->notifyHoverEnter();
	}
	else if (!newHoverComponentPtr && selectionHoverPtr)
	{
		selectionHoverPtr->notifyHoverExit();
		m_hoverComponentWeakPtr= SelectionComponentWeakPtr();
	}
}

void EditorObjectSystem::onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr currentHoverPtr= m_hoverComponentWeakPtr.lock();
	if (currentHoverPtr)
	{
		if (button == SDL_BUTTON_LEFT)
		{
			// See if the current selection is changing
			SelectionComponentPtr currentSelectedPtr = m_selectedComponentWeakPtr.lock();
			if (currentSelectedPtr != currentHoverPtr)
			{
				// Update the selection component weak ptr
				m_selectedComponentWeakPtr = m_hoverComponentWeakPtr;

				// Send notification of selection change
				onSelectionChanged(currentSelectedPtr, currentHoverPtr);
			}
		}

		if (currentHoverPtr->OnInteractionRayPress)
			currentHoverPtr->OnInteractionRayPress(button);
	}
}

void EditorObjectSystem::onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr selectionHoverPtr = m_hoverComponentWeakPtr.lock();
	if (selectionHoverPtr)
	{
		if (selectionHoverPtr->OnInteractionRayRelease)
			selectionHoverPtr->OnInteractionRayPress(button);
	}
}

void EditorObjectSystem::onSelectionChanged(SelectionComponentPtr oldComponentPtr, SelectionComponentPtr newComponentPtr)
{
	// Tell the old selection that it's getting unselected
	if (oldComponentPtr)
		oldComponentPtr->notifyUnselected();

	// Tell the new selection that it's getting selected
	if (newComponentPtr)
		newComponentPtr->notifySelected();
}

SelectionComponentWeakPtr EditorObjectSystem::findClosestSelectionTarget(
	const glm::vec3& rayOrigin, 
	const glm::vec3& rayDir,
	ColliderRaycastHitResult& outRaycastResult) const
{
	SelectionComponentWeakPtr closestComponent;
	
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
			closestComponent= selectionComponentWeakPtr;
			outRaycastResult= result;
		}
	}

	return closestComponent;
}