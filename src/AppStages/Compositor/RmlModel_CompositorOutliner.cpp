#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "MikanObject.h"
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
	StencilObjectSystemPtr stencilSystemPtr)
{
	m_anchorSystemPtr= anchorSystemPtr;
	m_stencilSystemPtr= stencilSystemPtr;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_outliner");
	if (!constructor)
		return false;

	// One time data model types registration
	if (!s_bHasRegisteredTypes)
	{
		// One time registration for compositor layer struct.
		if (auto layer_model_handle = constructor.RegisterStruct<RmlModel_CompositorComponent>())
		{
			layer_model_handle.RegisterMember("name", &RmlModel_CompositorComponent::name);
			layer_model_handle.RegisterMember("depth", &RmlModel_CompositorComponent::depth);
		}

		// One time registration for an array of stencil quads.
		constructor.RegisterArray<decltype(m_componentOutliner)>();

		s_bHasRegisteredTypes = true;
	}

	// Register Data Model Fields
	constructor.Bind("components", &m_componentOutliner);

	// Bind data model callbacks

	// Listen for anchor config changes
	m_anchorSystemPtr->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &RmlModel_CompositorOutliner::anchorSystemConfigMarkedDirty);

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

	m_modelHandle.DirtyVariable("components");
}

void RmlModel_CompositorOutliner::addSceneComponent(SceneComponentPtr sceneComponentPtr, int depth)
{
	const std::string& componentName= sceneComponentPtr->getName();
	RmlModel_CompositorComponent object = {componentName.empty() ? "<No Name>" : componentName, depth};
	m_componentOutliner.push_back(object);

	for (SceneComponentWeakPtr childSceneComponentWeakPtr : sceneComponentPtr->getChildComponents())
	{
		SceneComponentPtr childSceneComponentPtr= childSceneComponentWeakPtr.lock();
		
		if (childSceneComponentPtr)
		{
			addSceneComponent(childSceneComponentPtr, depth + 1);
		}
	}
}