#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "StencilObjectSystem.h"
#include "SceneComponent.h"
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
	constructor.BindEventCallback("select_object_entry", &RmlModel_CompositorOutliner::selectObjectEntry, this);

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty);

	// Listen for selection changes
	m_editorSystemPtr->OnSelectionChanged += 
		MakeDelegate(this, &RmlModel_CompositorOutliner::updateSelection);

	// Listen for stencil config changes
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::stencilSystemConfigMarkedDirty);

	// Fill in the data model
	rebuildComponentList();

	return true;
}

void RmlModel_CompositorOutliner::dispose()
{
	m_stencilSystemPtr->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::stencilSystemConfigMarkedDirty);

	m_editorSystemPtr->OnSelectionChanged -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::updateSelection);

	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty);

	RmlModel::dispose();
}

void RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		rebuildComponentList();
	}
}

void RmlModel_CompositorOutliner::stencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_quadStencilListPropertyId) || 
		changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_boxStencilListPropertyId) ||
		changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_modelStencilListPropertyId))
	{
		rebuildComponentList();
	}
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
	MikanObjectPtr ownerObject= sceneComponentPtr->getOwnerObject();
	if (ownerObject->getRootComponent() == sceneComponentPtr)
	{
		const std::string& objectName= ownerObject->getName();
		SelectionComponentPtr selectionComponent= ownerObject->getComponentOfType<SelectionComponent>();

		RmlModel_CompositorObject object = {objectName.empty() ? "<No Name>" : objectName, depth, selectionComponent};
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