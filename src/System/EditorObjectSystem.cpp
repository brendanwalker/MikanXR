#include "AnchorObjectSystem.h"
#include "App.h"
#include "EditorObjectSystem.h"
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
}

void EditorObjectSystem::dispose()
{
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
	SelectionComponentPtr selectionHoverPtr= m_selectionHoverWeakPtr.lock();

	if (newHoverComponentPtr && !selectionHoverPtr)
	{
		m_selectionHoverWeakPtr= newHoverComponentWeakPtr;

		if (newHoverComponentPtr->OnInteractionRayOverlapEnter)
			newHoverComponentPtr->OnInteractionRayOverlapEnter();
	}
	else if (!newHoverComponentPtr && selectionHoverPtr)
	{
		if (selectionHoverPtr->OnInteractionRayOverlapExit)
			selectionHoverPtr->OnInteractionRayOverlapExit();

		m_selectionHoverWeakPtr= SelectionComponentWeakPtr();
	}
}

void EditorObjectSystem::onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr selectionHoverPtr= m_selectionHoverWeakPtr.lock();
	if (selectionHoverPtr)
	{
		if (selectionHoverPtr->OnInteractionRayPress)
			selectionHoverPtr->OnInteractionRayPress(button);
	}
}

void EditorObjectSystem::onMouseRayButtonUp(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr selectionHoverPtr = m_selectionHoverWeakPtr.lock();
	if (selectionHoverPtr)
	{
		if (selectionHoverPtr->OnInteractionRayRelease)
			selectionHoverPtr->OnInteractionRayPress(button);
	}
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