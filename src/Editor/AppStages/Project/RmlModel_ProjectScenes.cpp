#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "EditorObjectSystem.h"
#include "MikanObject.h"
#include "ModelStencilComponent.h"
#include "QuadStencilComponent.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"
#include "StencilComponent.h"
#include "TransformComponent.h"
#include "StencilObjectSystemConfig.h"
#include "RmlModel_ProjectScenes.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_ProjectScenes::s_bHasRegisteredTypes = false;

bool RmlModel_ProjectScenes::init(
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
	constructor.BindEventCallback("add_new_anchor",&RmlModel_ProjectScenes::addNewAnchor, this);
	constructor.BindEventCallback("add_new_quad",&RmlModel_ProjectScenes::addNewQuad, this);
	constructor.BindEventCallback("add_new_box",&RmlModel_ProjectScenes::addNewBox, this);
	constructor.BindEventCallback("add_new_model",&RmlModel_ProjectScenes::addNewModel, this);
	constructor.BindEventCallback("select_object_entry", &RmlModel_ProjectScenes::selectObjectEntry, this);

	// Listen for anchor changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_ProjectScenes::anchorSystemConfigMarkedDirty);
	m_anchorSystemPtr->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	m_anchorSystemPtr->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	// Listen for selection changes
	m_editorSystemPtr->OnSelectionChanged += 
		MakeDelegate(this, &RmlModel_ProjectScenes::updateSelection);

	// Listen for stencil changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	m_stencilSystemPtr->OnObjectInitialized +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	m_stencilSystemPtr->OnObjectDisposed +=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	// Fill in the data model
	rebuildComponentList();

	return true;
}

void RmlModel_ProjectScenes::dispose()
{
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty);
	m_stencilSystemPtr->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	m_stencilSystemPtr->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	m_editorSystemPtr->OnSelectionChanged -=
		MakeDelegate(this, &RmlModel_ProjectScenes::updateSelection);

	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_ProjectScenes::anchorSystemConfigMarkedDirty);
	m_anchorSystemPtr->OnObjectInitialized -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectInitialized);
	m_anchorSystemPtr->OnObjectDisposed -=
		MakeDelegate(this, &RmlModel_ProjectScenes::onObjectDisposed);

	RmlModel::dispose();
}

void RmlModel_ProjectScenes::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildComponentList();
	}
}

void RmlModel_ProjectScenes::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilComponentDefinition::k_parentAnchorPropertyId) ||
		changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildComponentList();
	}
}

void RmlModel_ProjectScenes::onObjectInitialized(
	MikanObjectSystemPtr objectSystemPtr, 
	MikanObjectPtr objectPtr)
{
	rebuildComponentList();
}

void RmlModel_ProjectScenes::onObjectDisposed(
	MikanObjectSystemPtr objectSystemPtr, 
	MikanObjectConstPtr objectPtr)
{
	rebuildComponentList();
}

void RmlModel_ProjectScenes::rebuildComponentList()
{
	m_componentOutliner.clear();

	// Add all root anchors to the outliner
	for (const auto it : m_anchorSystemPtr->getAnchorMap())
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
	for (const auto it : m_stencilSystemPtr->getQuadStencilMap())
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
	for (const auto it : m_stencilSystemPtr->getBoxStencilMap())
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
	for (const auto it : m_stencilSystemPtr->getModelStencilMap())
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

	m_modelHandle.DirtyVariable("objects");

	updateSelection();
}

void RmlModel_ProjectScenes::updateSelection()
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

void RmlModel_ProjectScenes::addTransformComponent(TransformComponentPtr transformComponentPtr, int depth)
{
	if (!transformComponentPtr || transformComponentPtr->getWasDisposed())
		return;

	MikanObjectPtr ownerObject= transformComponentPtr->getOwnerObject();
	if (ownerObject->getRootComponent() == transformComponentPtr)
	{
		const std::string& name= ownerObject->getRootComponent()->getName();
		SelectionComponentPtr selectionComponent= ownerObject->getComponentOfType<SelectionComponent>();

		RmlModel_CompositorObject object = {name.empty() ? "<No Name>" : name, depth, selectionComponent};
		m_componentOutliner.push_back(object);
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

void RmlModel_ProjectScenes::addNewAnchor(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	const glm::mat4 anchorXform = glm::mat4(1.f);

	AnchorObjectSystemConfigPtr anchorSystemConfig= m_anchorSystemPtr->getAnchorSystemConfig();
	const std::string newAnchorName= StringUtils::stringify("Anchor ", anchorSystemConfig->nextAnchorId);

	m_anchorSystemPtr->addNewAnchor(newAnchorName, anchorXform);
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

	StencilObjectSystemConfigPtr stencilSystemConfig = m_stencilSystemPtr->getStencilSystemConfig();
	quad.stencil_name= StringUtils::stringify("Quad ", stencilSystemConfig->nextStencilId);

	m_stencilSystemPtr->addNewQuadStencil(quad);
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

	StencilObjectSystemConfigPtr stencilSystemConfig= m_stencilSystemPtr->getStencilSystemConfig();
	box.stencil_name= StringUtils::stringify("Box ", stencilSystemConfig->nextStencilId);

	m_stencilSystemPtr->addNewBoxStencil(box);
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

	StencilObjectSystemConfigPtr stencilSystemConfig = m_stencilSystemPtr->getStencilSystemConfig();
	model.stencil_name= StringUtils::stringify("Model ", stencilSystemConfig->nextStencilId);

	m_stencilSystemPtr->addNewModelStencil(model);
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
	if (toggle_index >= m_componentOutliner.size())
		return;

	const RmlModel_CompositorObject& selectedObject = m_componentOutliner[toggle_index];
	SelectionComponentPtr selectionComponent= selectedObject.selectionComponent.lock();
	if (selectionComponent)
	{
		m_editorSystemPtr->setSelection(selectionComponent);
	}
}