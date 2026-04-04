#pragma once

#include "ObjectSystemFwd.h"

// Forward declare all GuiPanel component types
class GuiPanel_MarkerObjectSystem;
class GuiPanel_AnchorComponent;
class GuiPanel_BoxStencilComponent;
class GuiPanel_CameraComponent;
class GuiPanel_ClientTextureSourceComponent;
class GuiPanel_CompositorComponent;
class GuiPanel_MarkerComponent;
class GuiPanel_MarkerTrackingVolumeComponent;
class GuiPanel_ModelStencilComponent;
class GuiPanel_NetworkVideoSourceComponent;
class GuiPanel_QuadStencilComponent;
class GuiPanel_SceneComponent;
class GuiPanel_SpoutTextureSourceComponent;
class GuiPanel_StageComponent;
class GuiPanel_TrackingMountComponent;
class GuiPanel_USBVideoSourceComponent;
class GuiPanel_VRTrackingVolumeComponent;

class ProjectGuiPanelContext
{
public:
	ProjectGuiPanelContext(class AppStage_Project* ownerAppStage);

	bool init();

	inline AppStage_Project* getOwnerAppStage() const { return m_ownerAppStage; }

	// System Panels
	inline GuiPanel_MarkerObjectSystem* getMarkerSystemPanel() const { return m_markerSystemPanel; }

	// Component Panels
	inline GuiPanel_AnchorComponent* getAnchorPanel() const { return m_anchorPanel; }
	inline GuiPanel_BoxStencilComponent* getBoxStencilPanel() const { return m_boxStencilPanel; }
	inline GuiPanel_CameraComponent* getCameraPanel() const { return m_cameraPanel; }
	inline GuiPanel_ClientTextureSourceComponent* getClientTextureSourcePanel() const { return m_clientTextureSourcePanel; }
	inline GuiPanel_CompositorComponent* getCompositorPanel() const { return m_compositorPanel; }
	inline GuiPanel_MarkerComponent* getMarkerPanel() const { return m_markerPanel; }
	inline GuiPanel_MarkerTrackingVolumeComponent* getMarkerTrackingVolumePanel() const { return m_markerTrackingVolumePanel; }
	inline GuiPanel_ModelStencilComponent* getModelStencilPanel() const { return m_modelStencilPanel; }
	inline GuiPanel_NetworkVideoSourceComponent* getNetworkVideoSourcePanel() const { return m_networkVideoSourcePanel; }
	inline GuiPanel_QuadStencilComponent* getQuadStencilPanel() const { return m_quadStencilPanel; }
	inline GuiPanel_SceneComponent* getScenePanel() const { return m_scenePanel; }
	inline GuiPanel_SpoutTextureSourceComponent* getSpoutTextureSourcePanel() const { return m_spoutTextureSourcePanel; }
	inline GuiPanel_StageComponent* getStagePanel() const { return m_stagePanel; }
	inline GuiPanel_TrackingMountComponent* getTrackingMountPanel() const { return m_trackingMountPanel; }
	inline GuiPanel_USBVideoSourceComponent* getUSBVideoSourcePanel() const { return m_usbVideoSourcePanel; }
	inline GuiPanel_VRTrackingVolumeComponent* getVRTrackingVolumePanel() const { return m_vrTrackingVolumePanel; }

private:
	class AppStage_Project* m_ownerAppStage = nullptr;

	// System Panels
	GuiPanel_MarkerObjectSystem* m_markerSystemPanel = nullptr;

	// Component Panels
	GuiPanel_AnchorComponent* m_anchorPanel = nullptr;
	GuiPanel_BoxStencilComponent* m_boxStencilPanel = nullptr;
	GuiPanel_CameraComponent* m_cameraPanel = nullptr;
	GuiPanel_ClientTextureSourceComponent* m_clientTextureSourcePanel = nullptr;
	GuiPanel_CompositorComponent* m_compositorPanel = nullptr;
	GuiPanel_MarkerComponent* m_markerPanel = nullptr;
	GuiPanel_MarkerTrackingVolumeComponent* m_markerTrackingVolumePanel = nullptr;
	GuiPanel_ModelStencilComponent* m_modelStencilPanel = nullptr;
	GuiPanel_NetworkVideoSourceComponent* m_networkVideoSourcePanel = nullptr;
	GuiPanel_QuadStencilComponent* m_quadStencilPanel = nullptr;
	GuiPanel_SceneComponent* m_scenePanel = nullptr;
	GuiPanel_SpoutTextureSourceComponent* m_spoutTextureSourcePanel = nullptr;
	GuiPanel_StageComponent* m_stagePanel = nullptr;
	GuiPanel_TrackingMountComponent* m_trackingMountPanel = nullptr;
	GuiPanel_USBVideoSourceComponent* m_usbVideoSourcePanel = nullptr;
	GuiPanel_VRTrackingVolumeComponent* m_vrTrackingVolumePanel = nullptr;
};
