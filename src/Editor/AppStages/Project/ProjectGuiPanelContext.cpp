#include "AppStage_Project.h"
#include "MarkerObjectSystem.h"
#include "ProjectGuiPanelContext.h"
#include "Shared/GuiPanel_AnchorComponent.h"
#include "Shared/GuiPanel_CameraComponent.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "Shared/GuiPanel_SceneComponent.h"
#include "Shared/GuiPanel_SpoutTextureSourceComponent.h"
#include "Shared/GuiPanel_StencilComponent.h"
#include "Shared/GuiPanel_StageComponent.h"
#include "Shared/GuiPanel_TrackingMountComponent.h"
#include "Shared/GuiPanel_USBVideoSourceComponent.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"

ProjectGuiPanelContext::ProjectGuiPanelContext(AppStage_Project* ownerAppStage)
	: m_ownerAppStage(ownerAppStage)
{
}

bool ProjectGuiPanelContext::init()
{
	// System Panels
	m_markerSystemPanel = m_ownerAppStage->addGuiPanel<GuiPanel_MarkerObjectSystem>();
	m_markerSystemPanel->init(m_ownerAppStage);
	m_markerSystemPanel->setObjectSystem(m_ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>());

	// Component Panels
	m_anchorPanel = m_ownerAppStage->addGuiPanel<GuiPanel_AnchorComponent>();
	m_anchorPanel->init(m_ownerAppStage);

	m_boxStencilPanel = m_ownerAppStage->addGuiPanel<GuiPanel_BoxStencilComponent>();
	m_boxStencilPanel->init(m_ownerAppStage);

	m_cameraPanel = m_ownerAppStage->addGuiPanel<GuiPanel_CameraComponent>();
	m_cameraPanel->init(m_ownerAppStage);

	m_clientTextureSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_ClientTextureSourceComponent>();
	m_clientTextureSourcePanel->init(m_ownerAppStage);

	m_compositorPanel = m_ownerAppStage->addGuiPanel<GuiPanel_CompositorComponent>();
	m_compositorPanel->init(m_ownerAppStage);

	m_markerPanel = m_ownerAppStage->addGuiPanel<GuiPanel_MarkerComponent>();
	m_markerPanel->init(m_ownerAppStage);

	m_markerTrackingVolumePanel = m_ownerAppStage->addGuiPanel<GuiPanel_MarkerTrackingVolumeComponent>();
	m_markerTrackingVolumePanel->init(m_ownerAppStage);

	m_modelStencilPanel = m_ownerAppStage->addGuiPanel<GuiPanel_ModelStencilComponent>();
	m_modelStencilPanel->init(m_ownerAppStage);

	m_networkVideoSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_NetworkVideoSourceComponent>();
	m_networkVideoSourcePanel->init(m_ownerAppStage);

	m_quadStencilPanel = m_ownerAppStage->addGuiPanel<GuiPanel_QuadStencilComponent>();
	m_quadStencilPanel->init(m_ownerAppStage);

	m_scenePanel = m_ownerAppStage->addGuiPanel<GuiPanel_SceneComponent>();
	m_scenePanel->init(m_ownerAppStage);

	m_spoutTextureSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_SpoutTextureSourceComponent>();
	m_spoutTextureSourcePanel->init(m_ownerAppStage);

	m_stagePanel = m_ownerAppStage->addGuiPanel<GuiPanel_StageComponent>();
	m_stagePanel->init(m_ownerAppStage);

	m_trackingMountPanel = m_ownerAppStage->addGuiPanel<GuiPanel_TrackingMountComponent>();
	m_trackingMountPanel->init(m_ownerAppStage);

	m_usbVideoSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_USBVideoSourceComponent>();
	m_usbVideoSourcePanel->init(m_ownerAppStage);

	m_vrTrackingVolumePanel = m_ownerAppStage->addGuiPanel<GuiPanel_VRTrackingVolumeComponent>();
	m_vrTrackingVolumePanel->init(m_ownerAppStage);

	return true;
}
