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
#include "StencilObjectSystem.h"
#include "StencilComponent.h"
#include "TransformComponent.h"
#include "StencilObjectSystemConfig.h"
#include "RmlModel_ProjectScenes.h"
#include "ProjectConfig.h"
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
	, m_selectedAnchorModel(std::make_shared<RmlModel_AnchorComponent>())
	, m_selectedBoxStencilModel(std::make_shared<RmlModel_BoxStencilComponent>())
	, m_selectedModelStencilModel(std::make_shared<RmlModel_ModelStencilComponent>())
	, m_selectedQuadStencilModel(std::make_shared<RmlModel_QuadStencilComponent>())
	, m_selectedSceneModel(std::make_shared<RmlModel_SceneComponent>())
{
}

bool RmlModel_ProjectScenes::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr,
	CompositorObjectSystemPtr compositorSystemPtr,
	EditorObjectSystemPtr editorSystemPtr,
	SceneObjectSystemPtr sceneSystemPtr,
	StageObjectSystemPtr stageSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr)
{
	m_anchorSystem= anchorSystemPtr;
	m_compositorSystem= compositorSystemPtr;
	m_editorSystem= editorSystemPtr;
	m_sceneSystem= sceneSystemPtr;
	m_stageSystem= stageSystemPtr;
	m_stencilSystem= stencilSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Scenes");
	if (!constructor)
		return false;

	// Register component lists
	m_stageIdList->init(
		constructor,
		stageSystemPtr->getStageSystemConfig(),
		StageObjectSystemConfig::k_stageListPropertyId,
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			auto stageSystemConfig = std::static_pointer_cast<StageObjectSystemConfig>(ownerConfig);

			for (const StageComponentDefinitionPtr stageComponent : stageSystemConfig->getStageList())
			{
				outComponentIdList.push_back((int)stageComponent->getStageId());
			}
		});
	m_sceneIdList->init(
		constructor,
		sceneSystemPtr->getSceneSystemConfig(),
		SceneObjectSystemConfig::k_sceneListPropertyId,
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			auto sceneSystemConfig = std::static_pointer_cast<SceneObjectSystemConfig>(ownerConfig);

			for (const SceneComponentDefinitionPtr sceneComponent : sceneSystemConfig->getSceneList())
			{
				if (sceneComponent->getParentStageId() == m_selectedStageId)
				{
					outComponentIdList.push_back((int)sceneComponent->getSceneId());
				}
			}
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

	// Register Selected Object Models
	m_selectedAnchorModel->init(rmlContext);
	m_selectedBoxStencilModel->init(rmlContext);
	m_selectedModelStencilModel->init(rmlContext);
	m_selectedQuadStencilModel->init(rmlContext);
	m_selectedSceneModel->init(rmlContext);

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
	anchorSystem->getAnchorSystemConfig()->OnMarkedDirty +=
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
	StencilObjectSystemPtr stencilSystem = m_stencilSystem.lock();
	stencilSystem->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	stencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	stencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	// Fill in the data model
	rebuildSceneComponentList();

	// Listen for stage/scene/compositor list changes
	m_stageIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectScenes::stageIdListChanged);
	m_sceneIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectScenes::sceneIdListChanged);

	return true;
}

void RmlModel_ProjectScenes::dispose()
{
	m_stageIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectScenes::stageIdListChanged);
	m_sceneIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectScenes::sceneIdListChanged);

	StencilObjectSystemPtr stencilSystem = m_stencilSystem.lock();
	stencilSystem->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	stencilSystem->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	stencilSystem->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	EditorObjectSystemPtr editorSystem = m_editorSystem.lock();
	editorSystem->OnSelectionChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::updateSelection);

	AnchorObjectSystemPtr anchorSystem = m_anchorSystem.lock();
	anchorSystem->getAnchorSystemConfig()->OnMarkedDirty -=
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
	for (const auto it : anchorSystem->getAnchorMap())
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

	// Add all root stencils to the outliner
	StencilObjectSystemPtr stencilSystem = m_stencilSystem.lock();
	for (const auto it : stencilSystem->getQuadStencilMap())
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
	for (const auto it : stencilSystem->getBoxStencilMap())
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
	for (const auto it : stencilSystem->getModelStencilMap())
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
		m_selectedSceneId = sceneId;
		m_modelHandle.DirtyVariable("selected_scene_id");

		if (auto sceneComponent = getSelectedSceneComponent())
		{
			m_selectedSceneModel->setComponent(sceneComponent);
		}
		else
		{
			m_selectedSceneModel->setComponent(nullptr);
		}

		rebuildSceneComponentList();
	}
}

void RmlModel_ProjectScenes::stageIdListChanged(bool bOwnerChanged)
{
	MikanStageID selectedStageId = INVALID_MIKAN_ID;
	if (!m_stageIdList->isEmpty() &&
		!m_stageIdList->contains(m_selectedStageId))
	{
		selectedStageId = m_stageIdList->getFirstValue();
	}

	// Defer the selection update to post view update after element list refreshes
	addModelUpdateCallback([this, selectedStageId]() {
		setSelectedStageId(selectedStageId);
	});
}

void RmlModel_ProjectScenes::sceneIdListChanged(bool bOwnerChanged)
{
	MikanSceneID selectedSceneId = INVALID_MIKAN_ID;
	if (!m_sceneIdList->isEmpty() &&
		!m_sceneIdList->contains(m_selectedStageId))
	{
		selectedSceneId = m_sceneIdList->getFirstValue();
	}

	// Defer the selection update to post view update after element list refreshes
	addModelUpdateCallback([this, selectedSceneId]() {
		setSelectedSceneId(selectedSceneId);
	});
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
	if (m_selectedSceneId != INVALID_MIKAN_ID)
	{
		m_sceneSystem.lock()->addNewScene(m_selectedSceneId);
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
	m_sceneSystem.lock()->removeScene(sceneId);
}

void RmlModel_ProjectScenes::addNewAnchor(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (m_selectedSceneId != INVALID_MIKAN_ID)
	{
		m_anchorSystem.lock()->addNewAnchor(m_selectedSceneId);
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
	m_anchorSystem.lock()->removeAnchor(anchorId);
}

void RmlModel_ProjectScenes::addNewQuad(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	MikanStencilQuadInfo quad = {};

	quad.is_double_sided = true;
	quad.parent_anchor_id = INVALID_MIKAN_ID;
	quad.relative_transform.position = {0.f, 0.f, 0.f};
	quad.relative_transform.rotation = {1.f, 0.f, 0.f, 0.f};
	quad.relative_transform.scale = {1.f, 1.f, 1.f};
	quad.quad_width = 0.25f;
	quad.quad_height = 0.25f;

	StencilObjectSystemPtr stencilSystem= m_stencilSystem.lock();
	StencilObjectSystemConfigPtr stencilSystemConfig = stencilSystem->getStencilSystemConfig();
	quad.stencil_name= StringUtils::stringify("Quad ", stencilSystemConfig->nextStencilId);

	stencilSystem->addNewQuadStencil(quad);
}

void RmlModel_ProjectScenes::removeQuad(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int quadStencilId = parameters[0].Get<int>();
	m_stencilSystem.lock()->removeQuadStencil(quadStencilId);
}

void RmlModel_ProjectScenes::addNewBox(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	MikanStencilBoxInfo box = {};

	box.parent_anchor_id = INVALID_MIKAN_ID;
	box.relative_transform.position = {0.f, 0.f, 0.f};
	box.relative_transform.rotation = {1.f, 0.f, 0.f, 0.f};
	box.relative_transform.scale = {1.f, 1.f, 1.f};
	box.box_x_size = 0.25f;
	box.box_y_size = 0.25f;
	box.box_z_size = 0.25f;

	StencilObjectSystemPtr stencilSystem = m_stencilSystem.lock();
	StencilObjectSystemConfigPtr stencilSystemConfig= stencilSystem->getStencilSystemConfig();
	box.stencil_name= StringUtils::stringify("Box ", stencilSystemConfig->nextStencilId);

	stencilSystem->addNewBoxStencil(box);
}

void RmlModel_ProjectScenes::removeBox(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int boxStencilId = parameters[0].Get<int>();
	m_stencilSystem.lock()->removeQuadStencil(boxStencilId);
}

void RmlModel_ProjectScenes::addNewModel(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	MikanStencilModelInfo model;

	model.is_disabled = false;
	model.parent_anchor_id = INVALID_MIKAN_ID;
	model.relative_transform.position = {0.f, 0.f, 0.f};
	model.relative_transform.rotation = {1.f, 0.f, 0.f, 0.f};
	model.relative_transform.scale = {1.f, 1.f, 1.f};

	StencilObjectSystemPtr stencilSystem = m_stencilSystem.lock();
	StencilObjectSystemConfigPtr stencilSystemConfig = stencilSystem->getStencilSystemConfig();
	model.stencil_name= StringUtils::stringify("Model ", stencilSystemConfig->nextStencilId);

	stencilSystem->addNewModelStencil(model);
}

void RmlModel_ProjectScenes::removeModel(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int modelStencilId = parameters[0].Get<int>();
	m_stencilSystem.lock()->removeModelStencil(modelStencilId);
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