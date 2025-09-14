#include "RmlModel_ProjectStages.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "MikanCoreTypes.h"
#include "ProjectConfig.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_ProjectStages::RmlModel_ProjectStages()
	: m_stageIdList(std::make_shared<RmlDataBinding_ComponentList>())
	, m_cameraIdList(std::make_shared<RmlDataBinding_ComponentList>())
	, m_compositorIdList(std::make_shared<RmlDataBinding_ComponentList>())
	, m_selectedStageModel(std::make_shared<RmlModel_PropertyInterface>())
	, m_selectedCameraModel(std::make_shared<RmlModel_PropertyInterface>())
	, m_selectedCompositorModel(std::make_shared<RmlModel_PropertyInterface>())
{
}

bool RmlModel_ProjectStages::init(
	Rml::Context* rmlContext, 
	ProjectConfigPtr projectConfig,
	StageObjectSystemPtr stageSystem,
	CameraObjectSystemPtr cameraSystem,
	CompositorObjectSystemPtr compositorSystem)
{
	StageObjectSystemConfigPtr stageConfig = projectConfig->stageConfig;

	m_projectConfig = projectConfig;
	m_stageSystem = stageSystem;
	m_cameraSystem = cameraSystem;
	m_compositorSystem = compositorSystem;

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "stages");
	if (!constructor)
		return false;

	// Register component lists
	m_stageIdList->init(
		constructor, 
		stageConfig,
		"stage_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			StageObjectSystemConfigPtr stageConfig = m_projectConfig.lock()->stageConfig;

			for (const auto& stagePtr : stageConfig->getStageList())
			{
				if (stagePtr)
				{
					outComponentIdList.push_back((int)stagePtr->getStageId());
				}
			}
		});

	m_cameraIdList->init(
		constructor,
		CommonConfigPtr(),
		"camera_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			if (ownerConfig)
			{
				auto stageDefinition = std::static_pointer_cast<StageComponentDefinition>(ownerConfig);
				
				CameraObjectSystemConfigPtr cameraConfig = m_projectConfig.lock()->cameraConfig;
				for (const auto& cameraPtr : cameraConfig->getCameraList())
				{
					if (cameraPtr && cameraPtr->getOwnerStageId() == stageDefinition->getStageId())
					{
						outComponentIdList.push_back((int)cameraPtr->getCameraId());
					}
				}
			}
		});

	m_compositorIdList->init(
		constructor,
		CommonConfigPtr(),
		"compositor_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			if (ownerConfig)
			{
				auto stageDefinition = std::static_pointer_cast<StageComponentDefinition>(ownerConfig);
				
				CompositorObjectSystemConfigPtr compositorConfig = m_projectConfig.lock()->compositorConfig;
				for (const auto& compositorPtr : compositorConfig->getCompositorList())
				{
					if (compositorPtr && compositorPtr->getOwnerStageId() == stageDefinition->getStageId())
					{
						outComponentIdList.push_back((int)compositorPtr->getCompositorId());
					}
				}
			}
		});

	// Register Data Model Fields
	constructor.Bind("selected_stage_id", &m_selectedStageId);
	constructor.Bind("selected_camera_id", &m_selectedCameraId);
	constructor.Bind("selected_compositor_id", &m_selectedCompositorId);

	// Register Selected Object Models
	m_selectedStageModel->init<StageComponent>(
		rmlContext,
		"stage_definition");
	m_selectedCameraModel->init<CameraComponent>(
		rmlContext,
		"camera_definition");
	m_selectedCompositorModel->init<CompositorComponent>(
		rmlContext,
		"compositor_definition");

	// Bind data model callbacks
	constructor.BindEventCallback("add_new_stage", &RmlModel_ProjectStages::addNewStage, this);
	constructor.BindEventCallback("remove_stage", &RmlModel_ProjectStages::removeStage, this);
	constructor.BindEventCallback("add_new_camera", &RmlModel_ProjectStages::addNewCamera, this);
	constructor.BindEventCallback("remove_camera", &RmlModel_ProjectStages::removeCamera, this);
	constructor.BindEventCallback("add_new_compositor", &RmlModel_ProjectStages::addNewCompositor, this);
	constructor.BindEventCallback("remove_compositor", &RmlModel_ProjectStages::removeCompositor, this);
	constructor.BindEventCallback("select_stage_entry", &RmlModel_ProjectStages::selectStageEntry, this);
	constructor.BindEventCallback("select_camera_entry", &RmlModel_ProjectStages::selectCameraEntry, this);
	constructor.BindEventCallback("select_compositor_entry", &RmlModel_ProjectStages::selectCompositorEntry, this);

	// Listen for config changes
	m_stageIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectStages::stageIdListChanged);
	m_cameraIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectStages::cameraIdListChanged);
	m_compositorIdList->OnChanged += MakeDelegate(this, &RmlModel_ProjectStages::compositorIdListChanged);

	return true;
}

void RmlModel_ProjectStages::dispose()
{
	m_stageIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectStages::stageIdListChanged);
	m_cameraIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectStages::cameraIdListChanged);
	m_compositorIdList->OnChanged -= MakeDelegate(this, &RmlModel_ProjectStages::compositorIdListChanged);

	m_selectedStageModel->dispose();
	m_selectedCameraModel->dispose();
	m_selectedCompositorModel->dispose();

	RmlModel::dispose();
}

void RmlModel_ProjectStages::stageIdListChanged(bool bOwnerChanged)
{
	MikanStageID selectedStageId = INVALID_MIKAN_ID;
	if (!m_stageIdList->isEmpty() &&
		!m_stageIdList->contains(m_selectedStageId))
	{
		selectedStageId = m_stageIdList->getComponentIdList()[0];
	}

	setSelectedStageId(selectedStageId);
}

void RmlModel_ProjectStages::cameraIdListChanged(bool bOwnerChanged)
{
	MikanCameraID selectedCameraId = INVALID_MIKAN_ID;
	if (!m_cameraIdList->isEmpty() &&
		!m_cameraIdList->contains(m_selectedCameraId))
	{
		selectedCameraId = m_cameraIdList->getComponentIdList()[0];
	}
	setSelectedCameraId(selectedCameraId);
}

void RmlModel_ProjectStages::compositorIdListChanged(bool bOwnerChanged)
{
	MikanCompositorID selectedCompositorId = INVALID_MIKAN_ID;
	if (!m_compositorIdList->isEmpty() &&
		!m_compositorIdList->contains(m_selectedCompositorId))
	{
		selectedCompositorId = m_compositorIdList->getComponentIdList()[0];
	}
	setSelectedCompositorId(selectedCompositorId);
}

StageObjectSystemPtr RmlModel_ProjectStages::getStageSystem()
{
	return m_stageSystem.lock();
}

CameraObjectSystemPtr RmlModel_ProjectStages::getCameraSystem()
{
	return m_cameraSystem.lock();
}

CompositorObjectSystemPtr RmlModel_ProjectStages::getCompositorSystem()
{
	return m_compositorSystem.lock();
}

StageComponentPtr RmlModel_ProjectStages::getSelectedStage()
{
	return getStageSystem()->getStageById((MikanStageID)m_selectedStageId);
}

CameraComponentPtr RmlModel_ProjectStages::getSelectedCamera()
{
	return getCameraSystem()->getCameraById((MikanCameraID)m_selectedCameraId);
}

CompositorComponentPtr RmlModel_ProjectStages::getSelectedCompositor()
{
	return getCompositorSystem()->getCompositorById((MikanCompositorID)m_selectedCompositorId);
}

void RmlModel_ProjectStages::addNewStage(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getStageSystem()->addNewStage();
}

void RmlModel_ProjectStages::removeStage(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int stageId = parameters[0].Get<int>();
	
	getStageSystem()->removeStage((MikanStageID)stageId);
}

void RmlModel_ProjectStages::addNewCamera(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{	
	// Would need GlmTransform parameter - this might need more specific implementation
	getCameraSystem()->addNewCamera((MikanStageID)m_selectedStageId);
}

void RmlModel_ProjectStages::removeCamera(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int cameraId = parameters[0].Get<int>();
	
	getCameraSystem()->removeCamera((MikanCameraID)cameraId);
}

void RmlModel_ProjectStages::addNewCompositor(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	getCompositorSystem()->addNewCompositor((MikanStageID)m_selectedStageId);
}

void RmlModel_ProjectStages::removeCompositor(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const int compositorId = parameters[0].Get<int>();
	
	getCompositorSystem()->removeCompositor((MikanCompositorID)compositorId);
}

void RmlModel_ProjectStages::selectStageEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const MikanStageID selectedStageId = (MikanStageID)parameters[0].Get<int>();
	setSelectedStageId(selectedStageId);
}

void RmlModel_ProjectStages::selectCameraEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const MikanCameraID selectedCameraId = (MikanCameraID)parameters[0].Get<int>();
	setSelectedCameraId(selectedCameraId);
}

void RmlModel_ProjectStages::selectCompositorEntry(
	Rml::DataModelHandle handle,
	Rml::Event& /*ev*/,
	const Rml::VariantList& parameters)
{
	if (parameters.empty())
		return;

	const MikanCompositorID selectedCompositorId = (MikanCompositorID)parameters[0].Get<int>();
	setSelectedCompositorId(selectedCompositorId);
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

			m_selectedStageModel->setPropertyInterface(stageComponent, stageComponent->getDefinition());
			
			m_cameraIdList->setOwnerConfig(stageDefinition);
			m_compositorIdList->setOwnerConfig(stageDefinition);
		}
		else
		{
			m_selectedStageModel->setPropertyInterface(nullptr);
			m_cameraIdList->setOwnerConfig(CommonConfigPtr());
			m_compositorIdList->setOwnerConfig(CommonConfigPtr());
		}
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
			m_selectedCameraModel->setPropertyInterface(cameraComponent, cameraComponent->getDefinition());
		}
		else
		{
			m_selectedCameraModel->setPropertyInterface(nullptr);
		}
	}
}

void RmlModel_ProjectStages::setSelectedCompositorId(MikanCompositorID compositorId)
{
	if (compositorId != m_selectedCompositorId)
	{
		m_selectedCompositorId = (int)compositorId;
		m_modelHandle.DirtyVariable("selected_compositor_id");

		if (CompositorComponentPtr compositorComponent = getSelectedCompositor())
		{
			m_selectedCompositorModel->setPropertyInterface(compositorComponent, compositorComponent->getDefinition());
		}
		else
		{
			m_selectedCompositorModel->setPropertyInterface(nullptr);
		}
	}
}