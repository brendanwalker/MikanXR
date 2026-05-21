#include "AnchorObjectSystem.h"
#include "App.h"
#include "Project/AppStage_Project.h"
#include "BoxColliderComponent.h"
#include "DiskColliderComponent.h"
#include "EditorObjectSystem.h"
#include "GizmoRotateComponent.h"
#include "GizmoScaleComponent.h"
#include "GizmoTransformComponent.h"
#include "GizmoTranslateComponent.h"
#include "MikanViewport.h"
#include "MikanPropertyDatabase.h"
#include "MikanFunctionDatabase.h"
#include "ObjectSystemColliderQueries.h"
#include "InputManager.h"
#include "IEditorWindow.h"
#include "ProjectManager.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanEditorTypes.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "StageComponent.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "QuadStencilSystem.h"
#include "BoxStencilSystem.h"
#include "ModelStencilSystem.h"
#include "Transform.h"

// -- AnchorObjectSystemConfig -----
const std::string EditorObjectSystemDefinition::k_renderOriginFlagPropertyId = "render_origin";
const std::string EditorObjectSystemDefinition::k_renderAnchorsPropertyId = "render_anchors";
const std::string EditorObjectSystemDefinition::k_renderQuadStencilsPropertyId = "render_quad_stencils";
const std::string EditorObjectSystemDefinition::k_renderBoxStencilsPropertyId = "render_box_stencils";
const std::string EditorObjectSystemDefinition::k_renderModelStencilsPropertyId = "render_model_stencils";
const std::string EditorObjectSystemDefinition::k_cameraSpeedPropertyId= "camera_speed";

configuru::Config EditorObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt[k_renderOriginFlagPropertyId] = m_editorSettings.bRenderOrigin;
	pt[k_renderAnchorsPropertyId] = m_editorSettings.bDebugRenderAnchors;
	pt[k_renderQuadStencilsPropertyId] = m_editorSettings.bDebugRenderQuadStencils;
	pt[k_renderBoxStencilsPropertyId] = m_editorSettings.bDebugRenderBoxStencils;
	pt[k_renderModelStencilsPropertyId] = m_editorSettings.bDebugRenderModelStencils;
	pt[k_cameraSpeedPropertyId] = m_editorSettings.cameraSpeed;

	return pt;
}

void EditorObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_editorSettings.bRenderOrigin = pt.get_or<bool>(k_renderOriginFlagPropertyId, m_editorSettings.bRenderOrigin);
	m_editorSettings.bDebugRenderAnchors = pt.get_or<bool>(k_renderAnchorsPropertyId, m_editorSettings.bDebugRenderAnchors);
	m_editorSettings.bDebugRenderQuadStencils = pt.get_or<bool>(k_renderQuadStencilsPropertyId, m_editorSettings.bDebugRenderQuadStencils);
	m_editorSettings.bDebugRenderBoxStencils = pt.get_or<bool>(k_renderBoxStencilsPropertyId, m_editorSettings.bDebugRenderBoxStencils);
	m_editorSettings.bDebugRenderModelStencils = pt.get_or<bool>(k_renderModelStencilsPropertyId, m_editorSettings.bDebugRenderModelStencils);
	m_editorSettings.cameraSpeed = pt.get_or<float>(k_cameraSpeedPropertyId, m_editorSettings.cameraSpeed);
}

void EditorObjectSystemDefinition::setRenderOriginFlag(bool flag)
{
	if (m_editorSettings.bRenderOrigin != flag)
	{
		m_editorSettings.bRenderOrigin = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderOriginFlagPropertyId));
	}
}

void EditorObjectSystemDefinition::setRenderAnchorsFlag(bool flag)
{
	if (m_editorSettings.bDebugRenderAnchors != flag)
	{
		m_editorSettings.bDebugRenderAnchors = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderAnchorsPropertyId));
	}
}

void EditorObjectSystemDefinition::setRenderQuadStencilsFlag(bool flag)
{
	if (m_editorSettings.bDebugRenderQuadStencils != flag)
	{
		m_editorSettings.bDebugRenderQuadStencils = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderQuadStencilsPropertyId));
	}
}

void EditorObjectSystemDefinition::setRenderBoxStencilsFlag(bool flag)
{
	if (m_editorSettings.bDebugRenderBoxStencils != flag)
	{
		m_editorSettings.bDebugRenderBoxStencils = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderBoxStencilsPropertyId));
	}
}

void EditorObjectSystemDefinition::setRenderModelStencilsFlag(bool flag)
{
	if (m_editorSettings.bDebugRenderModelStencils != flag)
	{
		m_editorSettings.bDebugRenderModelStencils = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderModelStencilsPropertyId));
	}
}

void EditorObjectSystemDefinition::setCameraSpeed(float speed)
{
	if (m_editorSettings.cameraSpeed != speed)
	{
		m_editorSettings.cameraSpeed = speed;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_cameraSpeedPropertyId));
	}
}

// -- EditorObjectSystem -----
bool EditorObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	auto editorConfig= getEditorSystemConfig();

	IEditorWindow* ownerWindow = getOwnerProjectManager()->getOwnerWindow();
	ownerWindow->OnAppStageEntered += MakeDelegate(this, &EditorObjectSystem::onAppStageEntered);

	SceneObjectSystemPtr sceneObjectSystem = getObjectSystemOfType<SceneObjectSystem>();
	sceneObjectSystem->OnSceneActivated += MakeDelegate(this, &EditorObjectSystem::onSceneActivated);
	sceneObjectSystem->OnSceneDeactivated += MakeDelegate(this, &EditorObjectSystem::onSceneDeactivated);

	AnchorObjectSystemPtr anchorObjectSystem= getObjectSystemOfType<AnchorObjectSystem>();
	anchorObjectSystem->OnComponentDisposed+= MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	QuadStencilSystemPtr quadStencilSystem= getObjectSystemOfType<QuadStencilSystem>();
	quadStencilSystem->OnComponentDisposed += MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	BoxStencilSystemPtr boxStencilSystem= getObjectSystemOfType<BoxStencilSystem>();
	boxStencilSystem->OnComponentDisposed += MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	ModelStencilSystemPtr modelStencilSystem= getObjectSystemOfType<ModelStencilSystem>();
	modelStencilSystem->OnComponentDisposed += MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	m_lastestRaycastResult = ColliderRaycastHitResult();
	m_hoverComponentWeakPtr.reset();
	m_selectedComponentWeakPtr.reset();

	return true;
}

void EditorObjectSystem::createSceneTransformGizmo(SceneComponentPtr ownerScene)
{
	disposeSceneTransformGizmo();

	m_gizmoObjectWeakPtr = newEmptyObject();
	MikanObjectPtr gizmoObjectPtr= m_gizmoObjectWeakPtr.lock();
	gizmoObjectPtr->setName("Gizmo");

	GizmoTransformComponentPtr transformGizmoPtr= gizmoObjectPtr->addComponent<GizmoTransformComponent>();
	transformGizmoPtr->setName("Gizmo");
	gizmoObjectPtr->setRootComponent(transformGizmoPtr);
	m_gizmoComponentWeakPtr= transformGizmoPtr;

	gizmoObjectPtr->addComponent<SelectionComponent>();

	const float W= GizmoTransformComponent::k_gizmoBaseWidth;
	const float R= GizmoTransformComponent::k_gizmoBaseRadius;
	const float P= R * 0.1f;

	GizmoTranslateComponentPtr translateComponentPtr= gizmoObjectPtr->addComponent<GizmoTranslateComponent>();
	createGizmoBoxCollider(gizmoObjectPtr, "xyTranslateHandle", glm::vec3(P, P, 0.f), glm::vec3(P, P, W * 0.1f), 3);
	createGizmoBoxCollider(gizmoObjectPtr, "xzTranslateHandle", glm::vec3(P, 0.f, P), glm::vec3(P, W * 0.1f, P), 3);
	createGizmoBoxCollider(gizmoObjectPtr, "yzTranslateHandle", glm::vec3(0.f, P, P), glm::vec3(W * 0.1f, P, P), 3);
	createGizmoDiskCollider(gizmoObjectPtr, "viewPlaneTranslateHandle", glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), W * 2.5f, 2);
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisTranslateHandle", glm::vec3(R/2.f, 0.f, 0.f), glm::vec3(R/2.f, W, W), 1);
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisTranslateHandle", glm::vec3(0.f, R/2.f, 0.f), glm::vec3(W, R/2.f, W), 1);
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisTranslateHandle", glm::vec3(0.f, 0.f, R/2.f), glm::vec3(W, W, R/2.f), 1);

	GizmoRotateComponentPtr rotateComponentPtr= gizmoObjectPtr->addComponent<GizmoRotateComponent>();
	createGizmoDiskCollider(gizmoObjectPtr, "xAxisRotateHandle", glm::vec3(0.f), glm::vec3(1.f, 0.f, 0.f), R, 1);
	createGizmoDiskCollider(gizmoObjectPtr, "yAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), R, 1);
	createGizmoDiskCollider(gizmoObjectPtr, "zAxisRotateHandle", glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f), R, 1);

	GizmoScaleComponentPtr scaleComponentPtr= gizmoObjectPtr->addComponent<GizmoScaleComponent>();
	createGizmoBoxCollider(gizmoObjectPtr, "centerScaleHandle", glm::vec3(0.f, 0.f, 0.f), glm::vec3(W, W, W), 1);
	createGizmoBoxCollider(gizmoObjectPtr, "xAxisScaleHandle", glm::vec3(R, 0.f, 0.f), glm::vec3(W, W, W), 1);
	createGizmoBoxCollider(gizmoObjectPtr, "yAxisScaleHandle", glm::vec3(0.f, R, 0.f), glm::vec3(W, W, W), 1);
	createGizmoBoxCollider(gizmoObjectPtr, "zAxisScaleHandle", glm::vec3(0.f, 0.f, R), glm::vec3(W, W, W), 1);

	gizmoObjectPtr->init();

	// Attach the gizmo to the scene
	gizmoObjectPtr->getRootComponent()->attachToComponent(ownerScene);
}

void EditorObjectSystem::disposeSceneTransformGizmo()
{
	MikanObjectPtr gizmoObjectPtr= m_gizmoObjectWeakPtr.lock();
	if (gizmoObjectPtr)
	{
		gizmoObjectPtr->dispose();
	}

	m_gizmoObjectWeakPtr.reset();
	m_gizmoComponentWeakPtr.reset();
}

void EditorObjectSystem::createGizmoBoxCollider(
	MikanObjectPtr gizmoObjectPtr,
	const std::string& name,
	const glm::vec3& center,
	const glm::vec3& halfExtents, 
	const int priority)
{
	BoxColliderComponentPtr colliderPtr= gizmoObjectPtr->addComponent<BoxColliderComponent>(name);

	colliderPtr->setName(name);
	colliderPtr->setHalfExtents(halfExtents);
	colliderPtr->setRelativeTransform(GlmTransform(center));
	colliderPtr->attachToComponent(gizmoObjectPtr->getRootComponent());
	colliderPtr->setEnabled(false);
	colliderPtr->setPriority(priority);
}

void EditorObjectSystem::createGizmoDiskCollider(
	MikanObjectPtr gizmoObjectPtr,
	const std::string& name,
	const glm::vec3& center,
	const glm::vec3& normal,
	const float radius,
	const int priority)
{
	DiskColliderComponentPtr colliderPtr = gizmoObjectPtr->addComponent<DiskColliderComponent>(name);

	glm::quat orientation= glm::quat(glm::vec3(0.f, 1.f, 0.f), normal);
	colliderPtr->setRelativeTransform(GlmTransform(center, orientation));
	colliderPtr->attachToComponent(gizmoObjectPtr->getRootComponent());
	colliderPtr->setRadius(radius);
	colliderPtr->setName(name);
	colliderPtr->setEnabled(false);
	colliderPtr->setPriority(priority);
}

void EditorObjectSystem::dispose()
{
	SceneObjectSystemPtr sceneObjectSystem = getObjectSystemOfType<SceneObjectSystem>();
	sceneObjectSystem->OnSceneActivated -= MakeDelegate(this, &EditorObjectSystem::onSceneActivated);
	sceneObjectSystem->OnSceneDeactivated -= MakeDelegate(this, &EditorObjectSystem::onSceneDeactivated);

	AnchorObjectSystemPtr anchorObjectSystem = getObjectSystemOfType<AnchorObjectSystem>();
	anchorObjectSystem->OnComponentDisposed -= MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	QuadStencilSystemPtr quadStencilSystem = getObjectSystemOfType<QuadStencilSystem>();
	quadStencilSystem->OnComponentDisposed -= MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	BoxStencilSystemPtr boxStencilSystem = getObjectSystemOfType<BoxStencilSystem>();
	boxStencilSystem->OnComponentDisposed -= MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	ModelStencilSystemPtr modelStencilSystem = getObjectSystemOfType<ModelStencilSystem>();
	modelStencilSystem->OnComponentDisposed -= MakeDelegate(this, &EditorObjectSystem::onActorDisposed);

	m_gizmoObjectWeakPtr.reset();
	m_gizmoComponentWeakPtr.reset();
	
	MikanObjectSystem::dispose();
}

void EditorObjectSystem::customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera)
{
	GizmoTransformComponentPtr gizmoComponent= m_gizmoComponentWeakPtr.lock();
	if (gizmoComponent)
	{
		gizmoComponent->customRender(graphicsContext, viewportCamera);
	}
}

EditorObjectSystemDefinitionConstPtr EditorObjectSystem::getEditorSystemConfigConst() const
{
	auto projectConfig= getProjectConfig();

	return projectConfig ? projectConfig->editorConfig : EditorObjectSystemDefinitionConstPtr();
}

EditorObjectSystemDefinitionPtr EditorObjectSystem::getEditorSystemConfig()
{
	return std::const_pointer_cast<EditorObjectSystemDefinition>(getEditorSystemConfigConst());
}

MikanComponentPtr EditorObjectSystem::getComponentById(int componentId) const
{
	// EditorObjectSystem doesn't manage components by ID
	return MikanComponentPtr();
}

bool EditorObjectSystem::getComponentList(const std::string& componentClassName, std::vector<MikanComponentPtr>& outComponentList) const
{
	// EditorObjectSystem doesn't manage ownership of components
	return false;
}

bool EditorObjectSystem::getComponentIdList(const std::string& componentClassName, std::vector<int>& outComponentIdList) const
{
	// EditorObjectSystem doesn't manage ownership of components
	return false;
}

MikanCameraPtr EditorObjectSystem::getPrimaryCamera() const
{
	if (!m_viewports.empty())
		if (auto viewport = m_viewports[0].lock())
			return viewport->getCurrentMikanCamera();
	return nullptr;
}

void EditorObjectSystem::bindViewport(MikanViewportWeakPtr viewportWeakPtr)
{
	MikanViewportPtr viewportPtr= viewportWeakPtr.lock();
	if (viewportPtr)
	{
		const auto it = std::find_if(
			m_viewports.begin(), m_viewports.end(),
			[viewportPtr](const MikanViewportWeakPtr& entry) {
				return entry.lock() == viewportPtr;
			});
		if (it == m_viewports.end())
		{
			viewportPtr->OnMouseExited += MakeDelegate(this, &EditorObjectSystem::onMouseExited);
			viewportPtr->OnMouseRayChanged += MakeDelegate(this, &EditorObjectSystem::onMouseRayChanged);
			viewportPtr->OnMouseRayButtonUp += MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonUp);
			viewportPtr->OnMouseRayButtonDown += MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonDown);

			m_viewports.push_back(viewportWeakPtr);
		}
	}
}

void EditorObjectSystem::unbindViewport(MikanViewportWeakPtr viewportWeakPtr)
{
	MikanViewportPtr viewportPtr = viewportWeakPtr.lock();
	if (viewportPtr)
	{
		const auto it = std::find_if(
			m_viewports.begin(), m_viewports.end(),
			[viewportPtr](const MikanViewportWeakPtr& entry) {
				return entry.lock() == viewportPtr;
			});
		if (it != m_viewports.end())
		{
			viewportPtr->OnMouseExited -= MakeDelegate(this, &EditorObjectSystem::onMouseExited);
			viewportPtr->OnMouseRayChanged -= MakeDelegate(this, &EditorObjectSystem::onMouseRayChanged);
			viewportPtr->OnMouseRayButtonUp -= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonUp);
			viewportPtr->OnMouseRayButtonDown -= MakeDelegate(this, &EditorObjectSystem::onMouseRayButtonDown);

			m_viewports.erase(it);
		}
	}
}

void EditorObjectSystem::clearViewports()
{
	for (MikanViewportWeakPtr& viewportWeakPtr : m_viewports)
	{
		MikanViewportPtr viewportPtr = viewportWeakPtr.lock();
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
void EditorObjectSystem::onAppStageEntered(class AppStage* oldAppStage, class AppStage* newAppStage)
{
	if (newAppStage->getAppStageName() == AppStage_Project::APP_STAGE_NAME)
	{
		InputManager* inputManager= getOwnerWindow()->getInputManager();
		inputManager->fetchOrAddKeyBindings(MkKey::DELETE_KEYCODE)->OnKeyPressed +=
			MakeDelegate(this, &EditorObjectSystem::onDeletePressed);

		m_gizmoComponentWeakPtr.lock()->bindInput();
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
		selectedComponent->destroyOwnerObject();
	}
}

// Object System Events
void EditorObjectSystem::onSceneActivated(SceneComponentPtr newScene)
{
	createSceneTransformGizmo(newScene);
}

void EditorObjectSystem::onSceneDeactivated(SceneComponentPtr oldScene)
{
	clearSelectedComponent();
	clearHoveredComponent();

	disposeSceneTransformGizmo();
}

void EditorObjectSystem::onActorDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component)
{
	SelectionComponentPtr hoverComponentPtr= m_hoverComponentWeakPtr.lock();
	if (hoverComponentPtr == component)
	{
		clearHoveredComponent();
	}

	SelectionComponentPtr selectedComponent= m_selectedComponentWeakPtr.lock();
	if (selectedComponent == component)
	{
		clearSelectedComponent();
	}
}

void EditorObjectSystem::onMouseExited()
{
	SelectionComponentPtr oldHoverComponentPtr = m_hoverComponentWeakPtr.lock();

	if (oldHoverComponentPtr)
	{
		oldHoverComponentPtr->notifyHoverExit(m_lastestRaycastResult);

		m_hoverComponentWeakPtr.reset();
		m_hoverColliderWeakPtr.reset();
		m_lastestRaycastResult = ColliderRaycastHitResult();
	}
}

void EditorObjectSystem::onMouseRayButtonDown(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)
{
	SelectionComponentPtr currentHoverPtr= m_hoverComponentWeakPtr.lock();

	if (button == MkMouseButton::LEFT)
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

	ColliderComponentPtr oldHoverColliderPtr = m_hoverColliderWeakPtr.lock();
	ColliderComponentPtr newHoverColliderPtr = m_lastestRaycastResult.hitComponent.lock();

	if (newHoverComponentPtr != oldHoverComponentPtr)
	{
		if (oldHoverComponentPtr)
			oldHoverComponentPtr->notifyHoverExit(prevRaycastResult);
		if (newHoverComponentPtr)
			newHoverComponentPtr->notifyHoverEnter(m_lastestRaycastResult);
		m_hoverComponentWeakPtr = newHoverComponentPtr;
	}
	else if (newHoverColliderPtr != oldHoverColliderPtr && oldHoverComponentPtr)
	{
		// Same SelectionComponent but different collider — fire exit+enter so
		// per-handle highlighting works (e.g. moving between gizmo handles).
		oldHoverComponentPtr->notifyHoverExit(prevRaycastResult);
		if (newHoverColliderPtr)
			oldHoverComponentPtr->notifyHoverEnter(m_lastestRaycastResult);
	}

	m_hoverColliderWeakPtr = newHoverColliderPtr;

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

void EditorObjectSystem::setObjectSystemSelectionFilter(
	const std::set<const MikanObjectSystem*>& objectSystemFilter)
{
	if (m_objectSystemSelectionFilter != objectSystemFilter)
	{
		// Flush any previous selection/hover since the filter is changing
		clearHoveredComponent();
		clearSelectedComponent();

		m_objectSystemSelectionFilter = objectSystemFilter;
	}
}

SelectionComponentPtr EditorObjectSystem::getSelectedSceneActor() const
{
	SelectionComponentPtr currentSelection = m_selectedComponentWeakPtr.lock();
	if (!currentSelection)
		return nullptr;

	// If the selected component belongs to the gizmo, return what the gizmo is manipulating instead
	MikanObjectPtr gizmoObject = m_gizmoObjectWeakPtr.lock();
	if (gizmoObject && currentSelection->getOwnerObject() == gizmoObject)
	{
		GizmoTransformComponentPtr gizmoComponent = m_gizmoComponentWeakPtr.lock();
		if (gizmoComponent)
			return gizmoComponent->getSelectionTarget();
		return nullptr;
	}

	return currentSelection;
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
	ColliderRaycastHitRequest request = {};
	request.rayOrigin = rayOrigin;
	request.rayDirection = rayDir;

	// Find the closest collision result in the 
	outRaycastResult = findClosestCollisionAlongRay(m_objectSystemSelectionFilter, request);

	SelectionComponentPtr closestSelectionComponent;
	ColliderComponentPtr hitCollider= outRaycastResult.hitComponent.lock();
	if (hitCollider)
	{
		MikanObjectPtr hitObject= hitCollider->getOwnerObject();

		closestSelectionComponent= hitObject->getComponentOfType<SelectionComponent>();
	}

	return closestSelectionComponent;
}

void EditorObjectSystem::clearHoveredComponent()
{
	SelectionComponentPtr hoverComponentPtr = m_hoverComponentWeakPtr.lock();
	if (hoverComponentPtr)
	{
		hoverComponentPtr->notifyHoverExit(m_lastestRaycastResult);
		m_hoverComponentWeakPtr.reset();
		m_hoverColliderWeakPtr.reset();
	}
}

void EditorObjectSystem::clearSelectedComponent()
{
	SelectionComponentPtr selectedComponent = m_selectedComponentWeakPtr.lock();
	if (selectedComponent)
	{
		// Clear the currently selected component first in case selection handler asks
		m_selectedComponentWeakPtr.reset();

		// Signal that the selection changed to nothing
		onSelectionChanged(selectedComponent, nullptr);
	}
}

void EditorObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<EditorObjectSystem>();
}

void EditorObjectSystem::registerFunctionDescriptors(MikanFunctionDatabasePtr functionDatabase)
{
	functionDatabase->registerFunctionsForSystem<EditorObjectSystem>();
}

// -- IEntityAccessor ----
rfk::Struct const* EditorObjectSystem::getClientAPIValuesStructType() const
{
	return &MikanEditorSystemValues::staticGetArchetype();
}

// -- IPropertyInterface ----
const std::string EditorObjectSystem::k_selectedLanguagePropertyId= "selected_language";
const std::string EditorObjectSystem::k_availableLanguageListPropertyId= "available_language_list";

void EditorObjectSystem::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_renderOriginFlagPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_renderAnchorsPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_renderQuadStencilsPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_renderBoxStencilsPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_renderModelStencilsPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_cameraSpeedPropertyId, MikanVariantType::FLOAT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystem::k_selectedLanguagePropertyId, MikanVariantType::STRING));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystem::k_availableLanguageListPropertyId, MikanVariantType::STRING_ARRAY)
		->setReadOnly());
}

bool EditorObjectSystem::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == EditorObjectSystemDefinition::k_renderOriginFlagPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getRenderOriginFlag();
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderAnchorsPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getRenderAnchorsFlag();
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderQuadStencilsPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getRenderQuadStencilsFlag();
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderBoxStencilsPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getRenderBoxStencilsFlag();
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderModelStencilsPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getRenderModelStencilsFlag();
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_cameraSpeedPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getCameraSpeed();
		return true;
	}
	else if (propertyName == EditorObjectSystem::k_selectedLanguagePropertyId)
	{
		outValue = getOwnerWindow()->getLocalizationManager()->getLanguage();
		return true;
	}
	else if (propertyName == EditorObjectSystem::k_availableLanguageListPropertyId)
	{
		outValue = getOwnerWindow()->getLocalizationManager()->getSupportedLanguages();
		return true;
	}

	return MikanObjectSystem::getPropertyValue(propertyName, outValue);
}

bool EditorObjectSystem::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == EditorObjectSystemDefinition::k_renderOriginFlagPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setRenderOriginFlag(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderAnchorsPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setRenderAnchorsFlag(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderQuadStencilsPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setRenderQuadStencilsFlag(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderBoxStencilsPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setRenderBoxStencilsFlag(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_renderModelStencilsPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setRenderModelStencilsFlag(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == EditorObjectSystemDefinition::k_cameraSpeedPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setCameraSpeed(inValue.getFloatValue());
		return true;
	}
	else if (propertyName == EditorObjectSystem::k_selectedLanguagePropertyId)
	{
		getOwnerWindow()->getLocalizationManager()->setLanguage(inValue.getStringValue());
		return true;
	}

	return MikanObjectSystem::setPropertyValue(propertyName, inValue);
}