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
#include "InputManager.h"
#include "ProjectManager.h"
#include "MainWindow.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "SceneObjectSystem.h"
#include "SceneComponent.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "QuadStencilSystem.h"
#include "BoxStencilSystem.h"
#include "ModelStencilSystem.h"
#include "Transform.h"

// -- AnchorObjectSystemConfig -----
const std::string EditorObjectSystemDefinition::k_cameraSpeedPropertyId= "cameraSpeed";
const std::string EditorObjectSystemDefinition::k_currentSceneNamePropertyId= "currentSceneName";

configuru::Config EditorObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["cameraSpeed"] = cameraSpeed;
	pt["currentSceneName"]= currentSceneName;

	return pt;
}

void EditorObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	cameraSpeed = pt.get_or<float>("cameraSpeed", cameraSpeed);
	currentSceneName = pt.get_or<std::string>("currentSceneName", currentSceneName);
}

void EditorObjectSystemDefinition::setCameraSpeed(float speed)
{
	if (cameraSpeed != speed)
	{
		cameraSpeed = speed;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_cameraSpeedPropertyId));
	}
}

void EditorObjectSystemDefinition::setCurrentSceneName(const std::string& sceneName)
{
	if (currentSceneName != sceneName)
	{
		currentSceneName = sceneName;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_currentSceneNamePropertyId));
	}
}

// -- EditorObjectSystem -----
bool EditorObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	auto editorConfig= getEditorSystemConfig();

	MainWindow::getInstance()->OnAppStageEntered += MakeDelegate(this, &EditorObjectSystem::onAppStageEntered);

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

bool EditorObjectSystem::getComponentIdList(const std::string& componentClassName, std::vector<int>& outComponentIdList) const
{
	// EditorObjectSystem doesn't manage ownership of components
	return false;
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
void EditorObjectSystem::onSceneActivated(SceneComponentPtr newScene)
{
	createSceneTransformGizmo(newScene);
}

void EditorObjectSystem::onSceneDeactivated(SceneComponentPtr oldScene)
{
	disposeSceneTransformGizmo();
}

void EditorObjectSystem::onActorDisposed(MikanObjectSystemPtr system, MikanComponentConstPtr component)
{
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

	SceneObjectSystemPtr sceneObjectSystem = getObjectSystemOfType<SceneObjectSystem>();
	SceneComponentConstPtr currentScene= sceneObjectSystem->getCurrentScene();
	if (currentScene)
	{
		closestSelectionComponent= 
			currentScene->findClosestSelectionTarget(
				rayOrigin, rayDir, outRaycastResult);
	}

	return closestSelectionComponent;
}

void EditorObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<EditorObjectSystem>();
}

// -- IPropertyInterface ----
void EditorObjectSystem::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			EditorObjectSystemDefinition::k_cameraSpeedPropertyId, MikanVariantType::FLOAT));
}

bool EditorObjectSystem::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == EditorObjectSystemDefinition::k_cameraSpeedPropertyId)
	{
		EditorObjectSystemDefinitionConstPtr definition = getEditorSystemConfigConst();
		outValue = definition->getCameraSpeed();
		return true;
	}

	return MikanObjectSystem::getPropertyValue(propertyName, outValue);
}

bool EditorObjectSystem::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	if (propertyName == EditorObjectSystemDefinition::k_cameraSpeedPropertyId)
	{
		EditorObjectSystemDefinitionPtr definition = getEditorSystemConfig();
		definition->setCameraSpeed(inValue.getFloatValue());
		return true;
	}

	return MikanObjectSystem::setPropertyValue(propertyName, inValue);
}