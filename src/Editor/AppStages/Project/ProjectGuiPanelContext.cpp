#include "AppStage_Project.h"
#include "DMXObjectSystem.h"
#include "MarkerObjectSystem.h"
#include "ProjectGuiPanelContext.h"
#include "Shared/GuiPanel_AnchorComponent.h"
#include "Shared/GuiPanel_CameraComponent.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "Shared/GuiPanel_DMXObjectSystem.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "Shared/GuiPanel_SceneComponent.h"
#include "Shared/GuiPanel_SpoutTextureSourceComponent.h"
#include "Shared/GuiPanel_RGBPixelGridComponent.h"
#include "Shared/GuiPanel_RGBSpotLightComponent.h"
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
	m_dmxSystemPanel = m_ownerAppStage->addGuiPanel<GuiPanel_DMXObjectSystem>();
	m_dmxSystemPanel->init();
	m_dmxSystemPanel->setObjectSystem(m_ownerAppStage->getObjectSystemOfType<DMXObjectSystem>());

	m_markerSystemPanel = m_ownerAppStage->addGuiPanel<GuiPanel_MarkerObjectSystem>();
	m_markerSystemPanel->init();
	m_markerSystemPanel->setObjectSystem(m_ownerAppStage->getObjectSystemOfType<MarkerObjectSystem>());

	// Component Panels
	m_anchorPanel = m_ownerAppStage->addGuiPanel<GuiPanel_AnchorComponent>();
	m_anchorPanel->init();

	m_boxStencilPanel = m_ownerAppStage->addGuiPanel<GuiPanel_BoxStencilComponent>();
	m_boxStencilPanel->init();

	m_cameraPanel = m_ownerAppStage->addGuiPanel<GuiPanel_CameraComponent>();
	m_cameraPanel->init();

	m_clientTextureSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_ClientTextureSourceComponent>();
	m_clientTextureSourcePanel->init();

	m_compositorPanel = m_ownerAppStage->addGuiPanel<GuiPanel_CompositorComponent>();
	m_compositorPanel->init();

	m_markerPanel = m_ownerAppStage->addGuiPanel<GuiPanel_MarkerComponent>();
	m_markerPanel->init();

	m_markerTrackingVolumePanel = m_ownerAppStage->addGuiPanel<GuiPanel_MarkerTrackingVolumeComponent>();
	m_markerTrackingVolumePanel->init();

	m_modelStencilPanel = m_ownerAppStage->addGuiPanel<GuiPanel_ModelStencilComponent>();
	m_modelStencilPanel->init();

	m_networkVideoSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_NetworkVideoSourceComponent>();
	m_networkVideoSourcePanel->init();

	m_pixelGridPanel = m_ownerAppStage->addGuiPanel<GuiPanel_RGBPixelGridComponent>();
	m_pixelGridPanel->init();

	m_quadStencilPanel = m_ownerAppStage->addGuiPanel<GuiPanel_QuadStencilComponent>();
	m_quadStencilPanel->init();

	m_spotLightPanel = m_ownerAppStage->addGuiPanel<GuiPanel_RGBSpotLightComponent>();
	m_spotLightPanel->init();

	m_scenePanel = m_ownerAppStage->addGuiPanel<GuiPanel_SceneComponent>();
	m_scenePanel->init();

	m_spoutTextureSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_SpoutTextureSourceComponent>();
	m_spoutTextureSourcePanel->init();

	m_stagePanel = m_ownerAppStage->addGuiPanel<GuiPanel_StageComponent>();
	m_stagePanel->init();

	m_trackingMountPanel = m_ownerAppStage->addGuiPanel<GuiPanel_TrackingMountComponent>();
	m_trackingMountPanel->init();

	m_usbVideoSourcePanel = m_ownerAppStage->addGuiPanel<GuiPanel_USBVideoSourceComponent>();
	m_usbVideoSourcePanel->init();

	m_vrTrackingVolumePanel = m_ownerAppStage->addGuiPanel<GuiPanel_VRTrackingVolumeComponent>();
	m_vrTrackingVolumePanel->init();

	return true;
}

void ProjectGuiPanelContext::update(float deltaSeconds)
{
	m_spoutTextureSourcePanel->update(deltaSeconds);
}