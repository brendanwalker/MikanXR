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
	Rml::Context* rmlContext= m_ownerAppStage->getRmlContext();

	// System Models
	m_markerSystemModel = m_ownerAppStage->addRmlModel<RmlModel_MarkerObjectSystem>();
	m_markerSystemModel->init(rmlContext);
	m_markerSystemModel->setObjectSystem(m_ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>());

	// Component Models
	m_anchorModel = m_ownerAppStage->addRmlModel<RmlModel_AnchorComponent>();
	m_anchorModel->init(rmlContext);

	m_boxStencilModel = m_ownerAppStage->addRmlModel<RmlModel_BoxStencilComponent>();
	m_boxStencilModel->init(rmlContext);

	m_cameraModel = m_ownerAppStage->addRmlModel<RmlModel_CameraComponent>();
	m_cameraModel->init(rmlContext);

	m_clientTextureSourceModel = m_ownerAppStage->addRmlModel<RmlModel_ClientTextureSourceComponent>();
	m_clientTextureSourceModel->init(rmlContext);

	m_compositorModel = m_ownerAppStage->addRmlModel<RmlModel_CompositorComponent>();
	m_compositorModel->init(rmlContext);

	m_markerModel = m_ownerAppStage->addRmlModel<RmlModel_MarkerComponent>();
	m_markerModel->init(rmlContext);

	m_markerTrackingVolumeModel = m_ownerAppStage->addRmlModel<RmlModel_MarkerTrackingVolumeComponent>();
	m_markerTrackingVolumeModel->init(rmlContext);

	m_modelStencilModel = m_ownerAppStage->addRmlModel<RmlModel_ModelStencilComponent>();
	m_modelStencilModel->init(rmlContext);

	m_networkVideoSourceModel = m_ownerAppStage->addRmlModel<RmlModel_NetworkVideoSourceComponent>();
	m_networkVideoSourceModel->init(rmlContext);

	m_quadStencilModel = m_ownerAppStage->addRmlModel<RmlModel_QuadStencilComponent>();
	m_quadStencilModel->init(rmlContext);

	m_sceneModel = m_ownerAppStage->addRmlModel<RmlModel_SceneComponent>();
	m_sceneModel->init(rmlContext);

	m_spoutTextureSourceModel = m_ownerAppStage->addRmlModel<RmlModel_SpoutTextureSourceComponent>();
	m_spoutTextureSourceModel->init(rmlContext);

	m_stageModel = m_ownerAppStage->addRmlModel<RmlModel_StageComponent>();
	m_stageModel->init(rmlContext);

	m_trackingMountModel = m_ownerAppStage->addRmlModel<RmlModel_TrackingMountComponent>();
	m_trackingMountModel->init(rmlContext);

	m_usbVideoSourceModel = m_ownerAppStage->addRmlModel<RmlModel_USBVideoSourceComponent>();
	m_usbVideoSourceModel->init(rmlContext);

	m_vrTrackingVolumeModel = m_ownerAppStage->addRmlModel<RmlModel_VRTrackingVolumeComponent>();
	m_vrTrackingVolumeModel->init(rmlContext);

	return true;
}