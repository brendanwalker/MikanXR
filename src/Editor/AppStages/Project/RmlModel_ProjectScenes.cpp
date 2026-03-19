#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "CompositorObjectSystem.h"
#include "CompositorComponent.h"
#include "EditorObjectSystem.h"
#include "MikanObject.h"
#include "ModelStencilComponent.h"
#include "MulticastDelegate.h"
#include "QuadStencilComponent.h"
#include "SelectionComponent.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "QuadStencilSystem.h"
#include "BoxStencilSystem.h"
#include "ModelStencilSystem.h"
#include "StencilComponent.h"
#include "SceneObjectSystem.h"
#include "TransformComponent.h"
#include "RmlModel_ProjectScenes.h"
#include "ProjectConfig.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "Shared/RmlModel_AnchorComponent.h"
#include "Shared/RmlModel_CompositorComponent.h"
#include "Shared/RmlModel_SceneComponent.h"
#include "Shared/RmlModel_StencilComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_ProjectScenes::s_bHasRegisteredTypes = false;

RmlModel_ProjectScenes::RmlModel_ProjectScenes()
	: m_stageIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_sceneIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{
}

bool RmlModel_ProjectScenes::init(ProjectRmlModelContext* context)
{
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_anchorSystem= ownerAppStage->getObjectSystemOfType<AnchorObjectSystem>();
	m_compositorSystem= ownerAppStage->getObjectSystemOfType<CompositorObjectSystem>();
	m_editorSystem= ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();
	m_sceneSystem= ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	m_stageSystem= ownerAppStage->getObjectSystemOfType<StageObjectSystem>();
	m_quadStencilSystem= ownerAppStage->getObjectSystemOfType<QuadStencilSystem>();
	m_boxStencilSystem= ownerAppStage->getObjectSystemOfType<BoxStencilSystem>();
	m_modelStencilSystem= ownerAppStage->getObjectSystemOfType<ModelStencilSystem>();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Scenes");
	if (!constructor)
		return false;

	// Register component lists
	m_stageIdList->init(
		constructor,
		m_stageSystem.lock(),
		StageObjectSystemDefinition::k_componentIdListPropertyId);
	m_sceneIdList->init(
		constructor,
		m_sceneSystem.lock()->getTypedDefinition(), // Listen for scene system definition changes
		"scene_ids", // virtual list that does not exist as a property
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			// Only show scenes whose parent stage matches the selected stage
			auto sceneSystemConfig = std::static_pointer_cast<SceneObjectSystemDefinition>(ownerConfig);

			for (const SceneComponentDefinitionPtr sceneComponent : sceneSystemConfig->getAllDefinitions())
			{
				if (sceneComponent->getParentStageId() == m_selectedStageId)
				{
					outComponentIdList.push_back((int)sceneComponent->getSceneId());
				}
			}
		},
		[this](const ConfigPropertyChangeSet& changedPropertySet) {
			// Refresh when the list of scene IDs changes on the scene system definition
			return changedPropertySet.hasPropertyName(SceneObjectSystemDefinition::k_componentIdListPropertyId);
		});

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_SceneObject>())
		{
			layer_model_handle.RegisterMember("name", &RmlModel_SceneObject::name);
			layer_model_handle.RegisterMember("depth", &RmlModel_SceneObject::depth);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_sceneOutliner)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("scene_objects", &m_sceneOutliner);
	constructor.Bind("selected_scene_object_index", &m_selectedSceneObjectIndex);
	constructor.Bind("selected_stage_id", &m_selectedStageId);
	constructor.Bind("selected_scene_id", &m_selectedSceneId);

	// Bind data model callbacks
	constructor.BindEventCallback("select_stage_entry", &RmlModel_ProjectScenes::selectStageEntry, this);
	constructor.BindEventCallback("select_scene_entry", &RmlModel_ProjectScenes::selectSceneEntry, this);
	constructor.BindEventCallback("add_new_scene", &RmlModel_ProjectScenes::addNewScene, this);
	constructor.BindEventCallback("remove_scene", &RmlModel_ProjectScenes::removeScene, this);
	constructor.BindEventCallback("add_new_anchor",&RmlModel_ProjectScenes::addNewAnchor, this);
	constructor.BindEventCallback("remove_anchor", &RmlModel_ProjectScenes::removeAnchor, this);
	constructor.BindEventCallback("add_new_quad",&RmlModel_ProjectScenes::addNewQuad, this);
	constructor.BindEventCallback("remove_quad", &RmlModel_ProjectScenes::removeQuad, this);
	constructor.BindEventCallback("add_new_box",&RmlModel_ProjectScenes::addNewBox, this);
	constructor.BindEventCallback("remove_box", &RmlModel_ProjectScenes::removeBox, this);
	constructor.BindEventCallback("add_new_model",&RmlModel_ProjectScenes::addNewModel, this);
	constructor.BindEventCallback("remove_model", &RmlModel_ProjectScenes::removeModel, this);
	constructor.BindEventCallback("select_object_entry", &RmlModel_ProjectScenes::selectObjectEntry, this);

	// Listen for anchor changes
	AnchorObjectSystemPtr anchorSystem = m_anchorSystem.lock();
	anchorSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &RmlModel_ProjectScenes::anchorSystemConfigMarkedDirty);
	anchorSystem->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	anchorSystem->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	// Listen for selection changes
	EditorObjectSystemPtr editorSystem = m_editorSystem.lock();
	editorSystem->OnSelectionChanged +=
		MakeDelegate(this, &RmlModel_ProjectScenes::updateSelection);

	// Listen for stencil changes
	QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();
	quadStencilSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	quadStencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	quadStencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();
	boxStencilSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	boxStencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	boxStencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();
	modelStencilSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	modelStencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	modelStencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	// Fill in the data model
	rebuildSceneComponentList();

	// Set initial selection to the current scene
	SceneComponentConstPtr currentScene= m_sceneSystem.lock()->getCurrentScene();
	if (currentScene)
	{
		MikanStageID parentStageId = currentScene->getParentStage()->getStageId();
		setSelectedStageId(parentStageId);
		setSelectedSceneId(currentScene->getSceneId());
	}

	// Listen for stage/scene/compositor list changes
	m_stageIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectScenes::stageIdListChanged);
	m_sceneIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectScenes::sceneIdListChanged);

	return true;
}

void RmlModel_ProjectScenes::dispose()
{
	m_stageIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectScenes::stageIdListChanged);
	m_sceneIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectScenes::sceneIdListChanged);

	QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();
	quadStencilSystem->getTypedDefinition()->OnPropertyChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	quadStencilSystem->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	quadStencilSystem->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();
	boxStencilSystem->getTypedDefinition()->OnPropertyChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	boxStencilSystem->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	boxStencilSystem->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();
	modelStencilSystem->getTypedDefinition()->OnPropertyChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	modelStencilSystem->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	modelStencilSystem->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	EditorObjectSystemPtr editorSystem = m_editorSystem.lock();
	editorSystem->OnSelectionChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::updateSelection);

	AnchorObjectSystemPtr anchorSystem = m_anchorSystem.lock();
	anchorSystem->getTypedDefinition()->OnPropertyChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::anchorSystemConfigMarkedDirty);
	anchorSystem->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	anchorSystem->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	RmlModel::dispose();
}

void RmlModel_ProjectScenes::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneComponentList();
	}
}

void RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilComponentDefinition::k_parentAnchorPropertyId) ||
		changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneComponentList();
	}
}

void RmlModel_ProjectScenes::onObjectInitialized(
	MikanObjectSystemPtr objectSystemPtr, 
	MikanObjectPtr objectPtr)
{
	rebuildSceneComponentList();
}

void RmlModel_ProjectScenes::onObjectDisposed(
	MikanObjectSystemPtr objectSystemPtr, 
	MikanObjectConstPtr objectPtr)
{
	rebuildSceneComponentList();
}

void RmlModel_ProjectScenes::rebuildSceneComponentList()
{
	m_sceneOutliner.clear();

	// Add all root anchors to the outliner
	AnchorObjectSystemPtr anchorSystem= m_anchorSystem.lock();
	for (const auto it : anchorSystem->getComponentMap())
	{
		AnchorComponentPtr anchorComponentPtr= it.second.lock();
		if (anchorComponentPtr)
		{
			TransformComponentPtr rootComponent= anchorComponentPtr->getOwnerObject()->getRootComponent();

			if (rootComponent->getParentComponent() == nullptr)
			{
				addTransformComponent(rootComponent, 0);
			}
		}
	}

	// Add all root quad stencils to the outliner
	QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();
	for (const auto it : quadStencilSystem->getComponentMap())
	{
		QuadStencilComponentPtr stencilComponentPtr = it.second.lock();
		if (stencilComponentPtr)
		{
			TransformComponentPtr rootComponent = stencilComponentPtr->getOwnerObject()->getRootComponent();

			if (rootComponent->getParentComponent() == nullptr)
			{
				addTransformComponent(rootComponent, 0);
			}
		}
	}

	// Add all root box stencils to the outliner
	BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();
	for (const auto it : boxStencilSystem->getComponentMap())
	{
		BoxStencilComponentPtr stencilComponentPtr = it.second.lock();
		if (stencilComponentPtr)
		{
			TransformComponentPtr rootComponent = stencilComponentPtr->getOwnerObject()->getRootComponent();

			if (rootComponent->getParentComponent() == nullptr)
			{
				addTransformComponent(rootComponent, 0);
			}
		}
	}

	// Add all root model stencils to the outliner
	ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();
	for (const auto it : modelStencilSystem->getComponentMap())
	{
		ModelStencilComponentPtr stencilComponentPtr= it.second.lock();
		if (stencilComponentPtr)
		{
			TransformComponentPtr rootComponent= stencilComponentPtr->getOwnerObject()->getRootComponent();

			if (rootComponent->getParentComponent() == nullptr)
			{
				addTransformComponent(rootComponent, 0);
			}
		}
	}

	m_modelHandle.DirtyVariable("scene_objects");

	updateSelection();
}

void RmlModel_ProjectScenes::updateSelection()
{
	// Find the index of the currently selected component (if any)
	m_selectedSceneObjectIndex = -1;
	SelectionComponentPtr currentSelection = m_editorSystem.lock()->getSelection();
	for (int list_index = 0; list_index < m_sceneOutliner.size(); ++list_index)
	{
		SelectionComponentPtr testComponentPtr = m_sceneOutliner[list_index].selectionComponent.lock();
		if (testComponentPtr == currentSelection)
		{
			m_selectedSceneObjectIndex = list_index;
			break;
		}
	}
	m_modelHandle.DirtyVariable("selected_scene_object_index");
}

void RmlModel_ProjectScenes::addTransformComponent(TransformComponentPtr transformComponentPtr, int depth)
{
	if (!transformComponentPtr || transformComponentPtr->getWasDisposed())
		return;

	MikanObjectPtr ownerObject= transformComponentPtr->getOwnerObject();
	if (ownerObject->getRootComponent() == transformComponentPtr)
	{
		const std::string& name= ownerObject->getRootComponent()->getName();
		SelectionComponentPtr selectionComponent= ownerObject->getComponentOfType<SelectionComponent>();

		RmlModel_SceneObject object = {name.empty() ? "<No Name>" : name, depth, selectionComponent};
		m_sceneOutliner.push_back(object);
	}

	for (TransformComponentWeakPtr childTransformComponentWeakPtr : transformComponentPtr->getChildComponents())
	{
		TransformComponentPtr childTransformComponentPtr= childTransformComponentWeakPtr.lock();
		
		if (childTransformComponentPtr)
		{
			int objectDepth= depth;

			if (childTransformComponentPtr->getOwnerObject() != ownerObject)
			{
				objectDepth++;
			}

			addTransformComponent(childTransformComponentPtr, objectDepth);
		}
	}
}

void RmlModel_ProjectScenes::setSelectedStageId(int stageId)
{
	if (stageId != m_selectedStageId)
	{
		m_selectedStageId = stageId;
		m_modelHandle.DirtyVariable("selected_stage_id");

		m_sceneIdList->rebuildList();
	}
}

void RmlModel_ProjectScenes::setSelectedSceneId(int sceneId)
{
	if (sceneId != m_selectedSceneId)
	{
		// Update the current scene in the scene system
		m_sceneSystem.lock()->setCurrentSceneById(sceneId);

		// Update the RML data model
		m_selectedSceneId = sceneId;
		m_modelHandle.DirtyVariable("selected_scene_id");

		if (auto sceneComponent = getSelectedSceneComponent())
		{
			m_projectRmlModelContext->getSceneModel()->setComponent(sceneComponent);
		}
		else
		{
			m_projectRmlModelContext->getSceneModel()->setComponent(nullptr);
		}

		rebuildSceneComponentList();
	}
}

void RmlModel_ProjectScenes::stageIdListChanged(bool bOwnerChanged)
{
	if (MikanStageID selectedStageId = m_selectedStageId;
		m_stageIdList->fixupSelectedValue(m_selectedStageId, INVALID_MIKAN_ID, selectedStageId))
	{
		// Defer the selection update to post view update after element list refreshes
		addModelUpdateCallback([this, selectedStageId]() {
			setSelectedStageId(selectedStageId);
		});
	}
}

void RmlModel_ProjectScenes::sceneIdListChanged(bool bOwnerChanged)
{
	if (MikanSceneID selectedSceneId = m_selectedSceneId;
		m_sceneIdList->fixupSelectedValue(m_selectedSceneId, INVALID_MIKAN_ID, selectedSceneId))
	{
		// Defer the selection update to post view update after element list refreshes
		addModelUpdateCallback([this, selectedSceneId]() {
			setSelectedSceneId(selectedSceneId);
		});
	}
}

StageComponentPtr RmlModel_ProjectScenes::getSelectedStageComponent()
{
	return m_stageSystem.lock()->getStageById(m_selectedStageId);
}

SceneComponentPtr RmlModel_ProjectScenes::getSelectedSceneComponent()
{
	return m_sceneSystem.lock()->getSceneById(m_selectedSceneId);
}

void RmlModel_ProjectScenes::selectStageEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int newStageId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedStageId(newStageId);
}

void RmlModel_ProjectScenes::selectSceneEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int newSceneId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedSceneId(newSceneId);
}

void RmlModel_ProjectScenes::addNewScene(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (m_selectedStageId != INVALID_MIKAN_ID)
	{
		m_sceneSystem.lock()->addNewObjectByTypedDefinition([this](auto sceneDefinition) {
			sceneDefinition->setParentStageId(m_selectedStageId);
			return true;
		});
	}
}

void RmlModel_ProjectScenes::removeScene(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int sceneId = parameters[0].Get<int>();
	m_sceneSystem.lock()->removeObjectByPrimaryComponentId(sceneId);
}

void RmlModel_ProjectScenes::addNewAnchor(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (m_selectedSceneId != INVALID_MIKAN_ID)
	{
		m_anchorSystem.lock()->addNewObjectByTypedDefinition(
			[this](auto anchorDefinition) {
				anchorDefinition->setOwnerSceneId(m_selectedSceneId);
				return true;
			});
	}
}

void RmlModel_ProjectScenes::removeAnchor(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int anchorId = parameters[0].Get<int>();
	m_anchorSystem.lock()->removeObjectByPrimaryComponentId(anchorId);
}

void RmlModel_ProjectScenes::addNewQuad(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();
	quadStencilSystem->addNewObjectByTypedDefinition([](QuadStencilDefinitionPtr definition) {

		// Initialize with default stencil info
		definition->setQuadWidth(0.25f);
		definition->setQuadHeight(0.25f);
		definition->setIsDoubleSided(true);
		definition->setRelativeTransform(GlmTransform());
		definition->setParentAnchorId(INVALID_MIKAN_ID);
		definition->setIsDisabled(false);

		return true;
	});
}

void RmlModel_ProjectScenes::removeQuad(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int quadStencilId = parameters[0].Get<int>();
	m_quadStencilSystem.lock()->removeObjectByPrimaryComponentId(quadStencilId);
}

void RmlModel_ProjectScenes::addNewBox(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();
	boxStencilSystem->addNewObjectByTypedDefinition([](BoxStencilDefinitionPtr definition) {

		// Initialize with default stencil info
		definition->setBoxXSize(0.25f);
		definition->setBoxYSize(0.25f);
		definition->setBoxZSize(0.25f);
		definition->setRelativeTransform(GlmTransform());
		definition->setParentAnchorId(INVALID_MIKAN_ID);
		definition->setIsDisabled(false);

		return true;
	});
}

void RmlModel_ProjectScenes::removeBox(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int boxStencilId = parameters[0].Get<int>();
	m_boxStencilSystem.lock()->removeObjectByPrimaryComponentId(boxStencilId);
}

void RmlModel_ProjectScenes::addNewModel(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();
	modelStencilSystem->addNewObjectByTypedDefinition([](ModelStencilDefinitionPtr definition) {

		// Initialize with default stencil info
		definition->setRelativeTransform(GlmTransform());
		definition->setParentAnchorId(INVALID_MIKAN_ID);
		definition->setIsDisabled(false);

		return true;
	});
}

void RmlModel_ProjectScenes::removeModel(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int modelStencilId = parameters[0].Get<int>();
	m_modelStencilSystem.lock()->removeObjectByPrimaryComponentId(modelStencilId);
}

void RmlModel_ProjectScenes::selectObjectEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	// The index of the file/directory being toggled is passed in as the first parameter.
	const size_t toggle_index = (size_t)parameters[0].Get<int>();
	if (toggle_index >= m_sceneOutliner.size())
		return;

	const RmlModel_SceneObject& selectedObject = m_sceneOutliner[toggle_index];
	SelectionComponentPtr selectionComponent= selectedObject.selectionComponent.lock();
	if (selectionComponent)
	{
		m_editorSystem.lock()->setSelection(selectionComponent);
	}
}