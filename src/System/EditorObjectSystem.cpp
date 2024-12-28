#include "AnchorObjectSystem.h"
#include "App.h"
#include "Compositor/AppStage_Compositor.h"
#include "BoxColliderComponent.h"
#include "DiskColliderComponent.h"
#include "EditorObjectSystem.h"
#include "GizmoRotateComponent.h"
#include "GizmoScaleComponent.h"
#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "GlViewport.h"
#include "InputManager.h"
#include "ObjectSystemManager.h"
#include "MainWindow.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanScene.h"
#include "ProfileConfig.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"

// -- AnchorObjectSystemConfig -----
configuru::Config EditorObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["cameraSpeed"] = cameraSpeed;

	return pt;
}

void EditorObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	cameraSpeed = pt.get_or<float>("cameraSpeed", cameraSpeed);
}

// -- EditorObjectSystem -----
EditorObjectSystemWeakPtr EditorObjectSystem::s_editorObjectSystem;

bool EditorObjectSystem::init()
{
	MikanObjectSystem::init();

	MainWindow::getInstance()->OnAppStageEntered += MakeDelegate(this, &EditorObjectSystem::onAppStageEntered);

	m_lastestRaycastResult= ColliderRaycastHitResult();
	m_hoverComponentWeakPtr.reset();
	m_selectedComponentWeakPtr.reset();

	m_scene = std::make_shared<MikanScene>();
	m_scene->init();

	ObjectSystemManagerPtr objSystemMgr= MainWindow::getInstance()->getObjectSystemManager();

	AnchorObjectSystemPtr anchorObjectSystem= objSystemMgr->getSystemOfType<AnchorObjectSystem>();
	anchorObjectSystem->OnComponentInitialized+= MakeDelegate(this, &EditorObjectSystem::onComponentInitialized);
	anchorObjectSystem->OnComponentDisposed+= MakeDelegate(this, &EditorObjectSystem::onComponentDisposed);
	for (MikanObjectPtr objectPtr : anchorObjectSystem->getObjectList())
	{
		m_scene->addMikanObject(objectPtr);
	}

	StencilObjectSystemPtr stencilObjectSystem= objSystemMgr->getSystemOfType<StencilObjectSystem>();
	stencilObjectSystem->OnComponentInitialized += MakeDelegate(this, &EditorObjectSystem::onComponentInitialized);
	stencilObjectSystem->OnComponentDisposed += MakeDelegate(this, &EditorObjectSystem::onComponentDisposed);
	for (MikanObjectPtr objectPtr : stencilObjectSystem->getObjectList())
	{
		m_scene->addMikanObject(objectPtr);
	}

	createTransformGizmo();

	s_editorObjectSystem = std::static_pointer_cast<EditorObjectSystem>(shared_from_this());
	return true;
}

void EditorObjectSystem::createTransformGizmo()
{
	m_gizmoObjectWeakPtr = newObject();
	MikanObjectPtr gizmoObjectPtr= m_gizmoObjectWeakPtr.lock();
	gizmoObjectPtr->setName("Gizmo");

	GizmoTransformComponentPtr transformGizmoPtr= gizmoObjectPtr->addComponent<GizmoTransformComponent>();
	transformGizmoPtr->setName("Gizmo");
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

	GizmoRotateComponentPtr rotateComponentPtr= gizmoObjectPtr->addComponent<GizmoRotateComponent>();
	createGizmoDiskCollider(gizmoObjectPtr, "xAxisRotateHandle", glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), R);
	createGizmoDiskCollider(gizmoObjectPtr, "yAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), R);
	createGizmoDiskCollider(gizmoObjectPtr, "zAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), R);

	GizmoScaleComponentPtr scaleComponentPtr= gizmoObjectPtr->addComponent<GizmoScaleComponent>();
	createGizmoBoxCollider(gizmoObjectPtr, "centerScaleHandle", glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.01f, 0.01f, 0.01f));
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisScaleHandle", glm::vec3(R/2.f, 0.f, 0.f), glm::vec3(W, W, W));
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisScaleHandle", glm::vec3(0.f, R/2.f, 0.f), glm::vec3(W, W, W));
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisScaleHandle", glm::vec3(0.f, 0.f, R/2.f), glm::vec3(W, W, W));

	gizmoObjectPtr->init();

	m_scene->addMikanObject(gizmoObjectPtr);
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
	colliderPtr->setPriority(1);
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
	colliderPtr->setPriority(1);
}

void EditorObjectSystem::dispose()
{
	s_editorObjectSystem.reset();

	m_gizmoObjectWeakPtr.reset();
	m_gizmoComponentWeakPtr.reset();
	m_scene= nullptr;

	MikanObjectSystem::dispose();
}

EditorObjectSystemConfigConstPtr EditorObjectSystem::getEditorSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->editorConfig;
}

EditorObjectSystemConfigPtr EditorObjectSystem::getEditorSystemConfig()
{
	return std::const_pointer_cast<EditorObjectSystemConfig>(getEditorSystemConfigConst());
}

void EditorObjectSystem::bindViewport(GlViewportWeakPtr viewportWeakPtr)
{
	GlViewportPtr viewportPtr= viewportWeakPtr.lock();
	if (!viewportPtr)
		return;

	viewportPtr->OnMouseExited+= MakeDelegate(this, &EditorObjectSystem::onMouseExited);
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
			viewportPtr->OnMouseExited-= MakeDelegate(this, &EditorObjectSystem::onMouseExited);
			viewportPtr->OnMouseRayChanged -= MakeDelegate(this, &EditorObjectSystem::onMouseRayChanged);
			viewportPtr->OnMouseRayButtonUp -= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonUp);
			viewportPtr->OnMouseRayButtonDown -= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonDown);
		}
	}
	m_viewports.clear();
}

// App Events
void EditorObjectSystem::onAppStageEntered(class AppStage* appStage)
{
	if (appStage->getAppStageName() == AppStage_Compositor::APP_STAGE_NAME)
	{
		m_gizmoComponentWeakPtr.lock()->bindInput();

		InputManager::getInstance()->fetchOrAddKeyBindings(SDLK_DELETE)->OnKeyPressed +=
			MakeDelegate(this, &EditorObjectSystem::onDeletePressed);
	}
}

// Keyboard Events
void EditorObjectSystem::onDeletePressed()
{
	SelectionComponentPtr selectedComponent= m_selectedComponentWeakPtr.lock();
	SelectionComponentPtr hoverComponentPtr= m_hoverComponentWeakPtr.lock();

	if (selectedComponent != nullptr)
	{
		// Clean up the config associated with owning object
		selectedComponent->getOwnerObject()->deleteSelfConfig();
	}
}

// Object System Events
void EditorObjectSystem::onComponentInitialized(MikanObjectSystemPtr system, MikanComponentPtr component)
{
	m_scene->addMikanComponent(component);
}

void EditorObjectSystem::onComponentDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component)
{
	m_scene->removeMikanComponent(component);

	SelectionComponentPtr hoverComponentPtr= m_hoverComponentWeakPtr.lock();
	if (hoverComponentPtr == component)
	{
		hoverComponentPtr->notifyHoverExit(m_lastestRaycastResult);
		m_hoverComponentWeakPtr.reset();
	}

	SelectionComponentPtr selectedComponent= m_selectedComponentWeakPtr.lock();
	if (selectedComponent == component)
	{
		// Clear the currently selected component first in case selection handler asks
		m_selectedComponentWeakPtr.reset();

		// Signal that the selection changed to nothing
		onSelectionChanged(selectedComponent, nullptr);
	}
}

void EditorObjectSystem::onMouseExited()
{
	SelectionComponentPtr oldHoverComponentPtr = m_hoverComponentWeakPtr.lock();

	if (oldHoverComponentPtr)
	{
		oldHoverComponentPtr->notifyHoverExit(m_lastestRaycastResult);

		m_hoverComponentWeakPtr.reset();
		m_lastestRaycastResult = ColliderRaycastHitResult();
	}
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

	if (newHoverComponentPtr != oldHoverComponentPtr)
	{
		if (oldHoverComponentPtr)
		{
			oldHoverComponentPtr->notifyHoverExit(prevRaycastResult);
		}

		if (newHoverComponentPtr)
		{
			newHoverComponentPtr->notifyHoverEnter(m_lastestRaycastResult);
		}

		m_hoverComponentWeakPtr = newHoverComponentPtr;
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
	GizmoTransformComponentPtr gizmoComponentPtr = m_gizmoComponentWeakPtr.lock();

	// Tell the old selection that it's getting unselected
	if (oldSelectedComponentPtr)
	{
		oldSelectedComponentPtr->notifyUnselected();
	}

	// Handle the new selection
	if (newSelectedComponentPtr)
	{
		// Tell the new selection that it's getting selected
		newSelectedComponentPtr->notifySelected();

		// Is the component selected not owned by the gizmo object?
		if (newSelectedComponentPtr->getOwnerObject() != gizmoComponentPtr->getOwnerObject())
		{
			SelectionComponentPtr oldGizmoTargetPtr= gizmoComponentPtr->getSelectionTarget();
			SelectionComponentPtr newGizmoTargetPtr = newSelectedComponentPtr;

			// Is the newly selected component not the one the transform gizmo is currently attached to?
			if (newGizmoTargetPtr && oldGizmoTargetPtr != newGizmoTargetPtr)
			{
				// Snap gizmo to the newly selected component
				gizmoComponentPtr->setSelectionTarget(newGizmoTargetPtr);
			}
		}
	}
	else
	{
		// Clean up the gizmo
		gizmoComponentPtr->clearSelectionTarget();
	}

	// Send an event for the selection changing
	if (OnSelectionChanged)
		OnSelectionChanged();
}

void EditorObjectSystem::setSelection(SelectionComponentPtr newSelectedComponentPtr)
{
	// See if the current selection is changing
	SelectionComponentPtr oldSelectedComponentPtr = m_selectedComponentWeakPtr.lock();
	if (oldSelectedComponentPtr != newSelectedComponentPtr)
	{
		// Update the selection component weak ptr
		m_selectedComponentWeakPtr = newSelectedComponentPtr;

		// Send notification of selection change
		onSelectionChanged(oldSelectedComponentPtr, newSelectedComponentPtr);
	}
}

SelectionComponentPtr EditorObjectSystem::findClosestSelectionTarget(
	const glm::vec3& rayOrigin, 
	const glm::vec3& rayDir,
	ColliderRaycastHitResult& outRaycastResult) const
{
	SelectionComponentPtr closestSelectionComponent;
	
	outRaycastResult.hitDistance= k_real_max;
	outRaycastResult.hitPriority= 0;
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
			(result.hitDistance < outRaycastResult.hitDistance || result.hitPriority > outRaycastResult.hitPriority))
		{
			closestSelectionComponent= selectionComponentPtr;
			outRaycastResult= result;
		}
	}

	return closestSelectionComponent;
}