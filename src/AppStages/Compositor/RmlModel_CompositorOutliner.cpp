#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"
#include "StencilComponent.h"
#include "SceneComponent.h"
#include "StencilObjectSystemConfig.h"
#include "RmlModel_CompositorOutliner.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorOutliner::s_bHasRegisteredTypes = false;

bool RmlModel_CompositorOutliner::init(
	Rml::Context* rmlContext,
	AnchorObjectSystemPtr anchorSystemPtr,
	EditorObjectSystemPtr editorSystemPtr,
	StencilObjectSystemPtr stencilSystemPtr)
{
	m_anchorSystemPtr= anchorSystemPtr;
	m_editorSystemPtr= editorSystemPtr;
	m_stencilSystemPtr= stencilSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_outliner");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorObject>())
		{
			layer_model_handle.RegisterMember("name", &RmlModel_CompositorObject::name);
			layer_model_handle.RegisterMember("depth", &RmlModel_CompositorObject::depth);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_componentOutliner)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("objects", &m_componentOutliner);
	constructor.Bind("selection_index", &m_selectionIndex);	

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_anchor",&RmlModel_CompositorOutliner::addNewAnchor, this);
	constructor.BindEventCallback("add_new_quad",&RmlModel_CompositorOutliner::addNewQuad, this);
	constructor.BindEventCallback("add_new_box",&RmlModel_CompositorOutliner::addNewBox, this);
	constructor.BindEventCallback("add_new_model",&RmlModel_CompositorOutliner::addNewModel, this);
	constructor.BindEventCallback("select_object_entry", &RmlModel_CompositorOutliner::selectObjectEntry, this);

	// Listen for anchor changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty);
	m_anchorSystemPtr->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectInitialized);
	m_anchorSystemPtr->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectDisposed);

	// Listen for selection changes
	m_editorSystemPtr->OnSelectionChanged += 
		MakeDelegate(this, &RmlModel_CompositorOutliner::updateSelection);

	// Listen for stencil changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::stencilSystemConfigMarkedDirty);
	m_stencilSystemPtr->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectInitialized);
	m_stencilSystemPtr->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectDisposed);

	// Fill in the data model
	rebuildComponentList();

	return true;
}

void RmlModel_CompositorOutliner::dispose()
{
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::stencilSystemConfigMarkedDirty);
	m_stencilSystemPtr->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectInitialized);
	m_stencilSystemPtr->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectDisposed);

	m_editorSystemPtr->OnSelectionChanged -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::updateSelection);

	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty);
	m_anchorSystemPtr->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectInitialized);
	m_anchorSystemPtr->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::onObjectDisposed);

	RmlModel::dispose();
}

void RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildComponentList();
	}
}

void RmlModel_CompositorOutliner::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilComponentDefinition::k_parentAnchorPropertyId) ||
		changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildComponentList();
	}
}

void RmlModel_CompositorOutliner::onObjectInitialized(
	MikanObjectSystemPtr objectSystemPtr, 
	MikanObjectPtr objectPtr)
{
	rebuildComponentList();
}

void RmlModel_CompositorOutliner::onObjectDisposed(
	MikanObjectSystemPtr objectSystemPtr, 
	MikanObjectConstPtr objectPtr)
{
	rebuildComponentList();
}

void RmlModel_CompositorOutliner::rebuildComponentList()
{
	SceneComponentPtr rootComponentPtr= m_anchorSystemPtr->getOriginSpatialAnchor();

	m_componentOutliner.clear();
	addSceneComponent(rootComponentPtr, 0);
	m_modelHandle.DirtyVariable("objects");

	updateSelection();
}

void RmlModel_CompositorOutliner::updateSelection()
{
	// Find the index of the currently selected component (if any)
	m_selectionIndex = -1;
	SelectionComponentPtr currentSelection = m_editorSystemPtr->getSelection();
	for (int list_index = 0; list_index < m_componentOutliner.size(); ++list_index)
	{
		SelectionComponentPtr testComponentPtr = m_componentOutliner[list_index].selectionComponent.lock();
		if (testComponentPtr == currentSelection)
		{
			m_selectionIndex = list_index;
			break;
		}
	}
	m_modelHandle.DirtyVariable("selection_index");
}

void RmlModel_CompositorOutliner::addSceneComponent(SceneComponentPtr sceneComponentPtr, int depth)
{
	if (!sceneComponentPtr || sceneComponentPtr->getWasDisposed())
		return;

	MikanObjectPtr ownerObject= sceneComponentPtr->getOwnerObject();
	if (ownerObject->getRootComponent() == sceneComponentPtr)
	{
		const std::string& name= ownerObject->getRootComponent()->getName();
		SelectionComponentPtr selectionComponent= ownerObject->getComponentOfType<SelectionComponent>();

		RmlModel_CompositorObject object = {name.empty() ? "<No Name>" : name, depth, selectionComponent};
		m_componentOutliner.push_back(object);
	}

	for (SceneComponentWeakPtr childSceneComponentWeakPtr : sceneComponentPtr->getChildComponents())
	{
		SceneComponentPtr childSceneComponentPtr= childSceneComponentWeakPtr.lock();
		
		if (childSceneComponentPtr)
		{
			int objectDepth= depth;

			if (childSceneComponentPtr->getOwnerObject() != ownerObject)
			{
				objectDepth++;
			}

			addSceneComponent(childSceneComponentPtr, objectDepth);
		}
	}
}

void RmlModel_CompositorOutliner::addNewAnchor(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	const glm::mat4 anchorXform = glm::mat4(1.f);
	char newAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];

	AnchorObjectSystemConfigPtr anchorSystemConfig= m_anchorSystemPtr->getAnchorSystemConfig();
	StringUtils::formatString(newAnchorName, sizeof(newAnchorName), "Anchor %d", anchorSystemConfig->nextAnchorId);

	m_anchorSystemPtr->addNewAnchor(newAnchorName, anchorXform);
}

void RmlModel_CompositorOutliner::addNewQuad(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	MikanStencilQuad quad;
	memset(&quad, 0, sizeof(MikanStencilQuad));

	quad.is_double_sided = true;
	quad.parent_anchor_id = INVALID_MIKAN_ID;
	quad.relative_transform.position = {0.f, 0.f, 0.f};
	quad.relative_transform.rotation = {1.f, 0.f, 0.f, 0.f};
	quad.relative_transform.scale = {1.f, 1.f, 1.f};
	quad.quad_width = 0.25f;
	quad.quad_height = 0.25f;

	StencilObjectSystemConfigPtr stencilSystemConfig = m_stencilSystemPtr->getStencilSystemConfig();
	StringUtils::formatString(quad.stencil_name, sizeof(quad.stencil_name), "Quad %d", stencilSystemConfig->nextStencilId);

	m_stencilSystemPtr->addNewQuadStencil(quad);
}

void RmlModel_CompositorOutliner::addNewBox(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	MikanStencilBox box;
	memset(&box, 0, sizeof(MikanStencilBox));

	box.parent_anchor_id = INVALID_MIKAN_ID;
	box.relative_transform.position = {0.f, 0.f, 0.f};
	box.relative_transform.rotation = {1.f, 0.f, 0.f, 0.f};
	box.relative_transform.scale = {1.f, 1.f, 1.f};
	box.box_x_size = 0.25f;
	box.box_y_size = 0.25f;
	box.box_z_size = 0.25f;

	StencilObjectSystemConfigPtr stencilSystemConfig= m_stencilSystemPtr->getStencilSystemConfig();
	StringUtils::formatString(box.stencil_name, sizeof(box.stencil_name), "Box %d", stencilSystemConfig->nextStencilId);

	m_stencilSystemPtr->addNewBoxStencil(box);
}

void RmlModel_CompositorOutliner::addNewModel(
	Rml::DataModelHandle handle, 
	Rml::Event& /*ev*/, 
	const Rml::VariantList& parameters)
{
	MikanStencilModel model;
	memset(&model, 0, sizeof(MikanStencilModel));

	model.is_disabled = false;
	model.parent_anchor_id = INVALID_MIKAN_ID;
	model.relative_transform.position = {0.f, 0.f, 0.f};
	model.relative_transform.rotation = {1.f, 0.f, 0.f, 0.f};
	model.relative_transform.scale = {1.f, 1.f, 1.f};

	StencilObjectSystemConfigPtr stencilSystemConfig = m_stencilSystemPtr->getStencilSystemConfig();
	StringUtils::formatString(model.stencil_name, sizeof(model.stencil_name), "Model %d", stencilSystemConfig->nextStencilId);

	m_stencilSystemPtr->addNewModelStencil(model);
}

void RmlModel_CompositorOutliner::selectObjectEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	// The index of the file/directory being toggled is passed in as the first parameter.
	const size_t toggle_index = (size_t)parameters[0].Get<int>();
	if (toggle_index >= m_componentOutliner.size())
		return;

	const RmlModel_CompositorObject& selectedObject = m_componentOutliner[toggle_index];
	SelectionComponentPtr selectionComponent= selectedObject.selectionComponent.lock();
	if (selectionComponent)
	{
		m_editorSystemPtr->setSelection(selectionComponent);
	}
}