#include "RmlModel_ProjectStages.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "MikanCoreTypes.h"
#include "ProjectConfig.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectRmlModelContext.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "Shared/RmlModel_CameraComponent.h"
#include "Shared/RmlModel_StageComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectStages::RmlModel_ProjectStages()
	: m_stageIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
	, m_cameraIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{
}

bool RmlModel_ProjectStages::init(ProjectRmlModelContext* context)
{
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	Rml::Context* rmlContext = ownerAppStage->getRmlContext();

	m_projectRmlModelContext = context;
	m_stageSystem = ownerAppStage->getObjectSystemOfType<StageObjectSystem>();
	m_cameraSystem = ownerAppStage->getObjectSystemOfType<CameraObjectSystem>();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "Stages");
	if (!constructor)
		return false;

	// Register component lists
	m_stageIdList->init(
		constructor,
		m_stageSystem.lock(),
		StageObjectSystemDefinition::k_componentIdListPropertyId);

	m_cameraIdList->init(
		constructor,
		m_cameraSystem.lock()->getTypedDefinition(),
		"camera_ids", // virtual list: only cameras owned by the selected stage
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			CameraObjectSystemDefinitionConstPtr cameraConfig =
				std::static_pointer_cast<CameraObjectSystemDefinition>(ownerConfig);
			
			for (const auto& cameraPtr : cameraConfig->getAllDefinitions())
			{
				if (cameraPtr && cameraPtr->getOwnerStageId() == m_selectedStageId)
				{
					outComponentIdList.push_back((int)cameraPtr->getComponentId());
				}
			}
		});

	// Register Data Model Fields
	constructor.Bind("selected_stage_id", &m_selectedStageId);
	constructor.Bind("selected_camera_id", &m_selectedCameraId);

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_stage", &RmlModel_ProjectStages::addNewStage, this);
	constructor.BindEventCallback("remove_stage", &RmlModel_ProjectStages::removeStage, this);
	constructor.BindEventCallback("add_new_camera", &RmlModel_ProjectStages::addNewCamera, this);
	constructor.BindEventCallback("remove_camera", &RmlModel_ProjectStages::removeCamera, this);
	constructor.BindEventCallback("select_stage_entry", &RmlModel_ProjectStages::selectStageEntry, this);
	constructor.BindEventCallback("select_camera_entry", &RmlModel_ProjectStages::selectCameraEntry, this);

	// Listen for config changes
	m_stageIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectStages::stageIdListChanged);
	m_cameraIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectStages::cameraIdListChanged);

	// Set initial state based on the current scene
	auto sceneSystem = ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	SceneComponentConstPtr currentScene = sceneSystem->getCurrentScene();
	if (currentScene)
	{
		MikanStageID parentStageId = currentScene->getParentStage()->getStageId();
		setSelectedStageId(parentStageId);
	}

	return true;
}

void RmlModel_ProjectStages::dispose()
{
	m_stageIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectStages::stageIdListChanged);
	m_cameraIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectStages::cameraIdListChanged);

	RmlModel::dispose();
}

void RmlModel_ProjectStages::stageIdListChanged(bool bOwnerChanged)
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

void RmlModel_ProjectStages::cameraIdListChanged(bool bOwnerChanged)
{
	if (MikanCameraID selectedCameraId = m_selectedCameraId;
		m_cameraIdList->fixupSelectedValue(m_selectedCameraId, INVALID_MIKAN_ID, selectedCameraId))
	{
		// Defer the selection update to post view update after element list refreshes
		addModelUpdateCallback([this, selectedCameraId]() {
			setSelectedCameraId(selectedCameraId);
		});
	}
}

StageObjectSystemPtr RmlModel_ProjectStages::getStageSystem()
{
	return m_stageSystem.lock();
}

CameraObjectSystemPtr RmlModel_ProjectStages::getCameraSystem()
{
	return m_cameraSystem.lock();
}

StageComponentPtr RmlModel_ProjectStages::getSelectedStage()
{
	return getStageSystem()->getStageById((MikanStageID)m_selectedStageId);
}

CameraComponentPtr RmlModel_ProjectStages::getSelectedCamera()
{
	return getCameraSystem()->getCameraById((MikanCameraID)m_selectedCameraId);
}

void RmlModel_ProjectStages::addNewStage(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getStageSystem()->addNewObjectByTypedDefinition();
}

void RmlModel_ProjectStages::removeStage(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getStageSystem()->removeObjectByPrimaryComponentId(m_selectedStageId);
}

void RmlModel_ProjectStages::addNewCamera(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{	
	getCameraSystem()->addNewObjectByTypedDefinition([this](CameraDefinitionPtr definition) {
		definition->setRelativeTransform(GlmTransform());
		definition->setOwnerStageId(m_selectedStageId);
		return true;
	});
}

void RmlModel_ProjectStages::removeCamera(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getCameraSystem()->removeObjectByPrimaryComponentId(m_selectedCameraId);
}

void RmlModel_ProjectStages::selectStageEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int newStageId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedStageId(newStageId);
}

void RmlModel_ProjectStages::selectCameraEntry(
	Rml::DataModelHandle handle,
	Rml::Event& ev,
	const Rml::VariantList& parameters)
{
	const int newCameraId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

	setSelectedCameraId(newCameraId);
}

void RmlModel_ProjectStages::setSelectedStageId(MikanStageID stageId)
{
	if (stageId != m_selectedStageId)
	{
		m_selectedStageId = (int)stageId;
		m_modelHandle.DirtyVariable("selected_stage_id");

		if (StageComponentPtr stageComponent = getSelectedStage())
		{
			StageComponentDefinitionPtr stageDefinition= stageComponent->getStageComponentDefinition();

			m_projectRmlModelContext->getStageModel()->setComponent(stageComponent);
		}
		else
		{
			m_projectRmlModelContext->getStageModel()->setComponent(nullptr);
		}

		m_cameraIdList->rebuildList();
	}
}

void RmlModel_ProjectStages::setSelectedCameraId(MikanCameraID cameraId)
{
	if (cameraId != m_selectedCameraId)
	{
		m_selectedCameraId = (int)cameraId;
		m_modelHandle.DirtyVariable("selected_camera_id");

		if (CameraComponentPtr cameraComponent = getSelectedCamera())
		{
			m_projectRmlModelContext->getCameraModel()->setComponent(cameraComponent);
		}
		else
		{
			m_projectRmlModelContext->getCameraModel()->setComponent(nullptr);
		}
	}
}