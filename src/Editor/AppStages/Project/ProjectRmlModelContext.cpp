#include "AppStage_Project.h"
#include "MarkerObjectSystem.h"
#include "ProjectRmlModelContext.h"
#include "Shared/RmlModel_AnchorComponent.h"
#include "Shared/RmlModel_CameraComponent.h"
#include "Shared/RmlModel_ClientTextureSourceComponent.h"
#include "Shared/RmlModel_CompositorComponent.h"
#include "Shared/RmlModel_MarkerObjectSystem.h"
#include "Shared/RmlModel_MarkerTrackingVolumeComponent.h"
#include "Shared/RmlModel_MarkerComponent.h"
#include "Shared/RmlModel_NetworkVideoSourceComponent.h"
#include "Shared/RmlModel_SceneComponent.h"
#include "Shared/RmlModel_SpoutTextureSourceComponent.h"
#include "Shared/RmlModel_StencilComponent.h"
#include "Shared/RmlModel_StageComponent.h"
#include "Shared/RmlModel_TrackingMountComponent.h"
#include "Shared/RmlModel_USBVideoSourceComponent.h"
#include "Shared/RmlModel_VRTrackingVolumeComponent.h"

ProjectRmlModelContext::ProjectRmlModelContext(AppStage_Project* ownerAppStage)
	: m_ownerAppStage(ownerAppStage)
{
}

bool ProjectRmlModelContext::init()
{
	// System Models
	m_markerSystemModel = m_ownerAppStage->addRmlModel<RmlModel_MarkerObjectSystem>();
	m_markerSystemModel->init(m_ownerAppStage);
	m_markerSystemModel->setObjectSystem(m_ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>());

	// Component Models
	m_anchorModel = m_ownerAppStage->addRmlModel<RmlModel_AnchorComponent>();
	m_anchorModel->init(m_ownerAppStage);

	m_boxStencilModel = m_ownerAppStage->addRmlModel<RmlModel_BoxStencilComponent>();
	m_boxStencilModel->init(m_ownerAppStage);

	m_cameraModel = m_ownerAppStage->addRmlModel<RmlModel_CameraComponent>();
	m_cameraModel->init(m_ownerAppStage);

	m_clientTextureSourceModel = m_ownerAppStage->addRmlModel<RmlModel_ClientTextureSourceComponent>();
	m_clientTextureSourceModel->init(m_ownerAppStage);

	m_compositorModel = m_ownerAppStage->addRmlModel<RmlModel_CompositorComponent>();
	m_compositorModel->init(m_ownerAppStage);

	m_markerModel = m_ownerAppStage->addRmlModel<RmlModel_MarkerComponent>();
	m_markerModel->init(m_ownerAppStage);

	m_markerTrackingVolumeModel = m_ownerAppStage->addRmlModel<RmlModel_MarkerTrackingVolumeComponent>();
	m_markerTrackingVolumeModel->init(m_ownerAppStage);

	m_modelStencilModel = m_ownerAppStage->addRmlModel<RmlModel_ModelStencilComponent>();
	m_modelStencilModel->init(m_ownerAppStage);

	m_networkVideoSourceModel = m_ownerAppStage->addRmlModel<RmlModel_NetworkVideoSourceComponent>();
	m_networkVideoSourceModel->init(m_ownerAppStage);

	m_quadStencilModel = m_ownerAppStage->addRmlModel<RmlModel_QuadStencilComponent>();
	m_quadStencilModel->init(m_ownerAppStage);

	m_sceneModel = m_ownerAppStage->addRmlModel<RmlModel_SceneComponent>();
	m_sceneModel->init(m_ownerAppStage);

	m_spoutTextureSourceModel = m_ownerAppStage->addRmlModel<RmlModel_SpoutTextureSourceComponent>();
	m_spoutTextureSourceModel->init(m_ownerAppStage);

	m_stageModel = m_ownerAppStage->addRmlModel<RmlModel_StageComponent>();
	m_stageModel->init(m_ownerAppStage);

	m_trackingMountModel = m_ownerAppStage->addRmlModel<RmlModel_TrackingMountComponent>();
	m_trackingMountModel->init(m_ownerAppStage);

	m_usbVideoSourceModel = m_ownerAppStage->addRmlModel<RmlModel_USBVideoSourceComponent>();
	m_usbVideoSourceModel->init(m_ownerAppStage);

	m_vrTrackingVolumeModel = m_ownerAppStage->addRmlModel<RmlModel_VRTrackingVolumeComponent>();
	m_vrTrackingVolumeModel->init(m_ownerAppStage);

	return true;
}