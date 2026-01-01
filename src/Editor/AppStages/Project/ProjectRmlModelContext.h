#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel.h"

class ProjectRmlModelContext
{
public:
	ProjectRmlModelContext(class AppStage_Project* ownerAppStage);

	bool init();

	inline AppStage_Project* getOwnerAppStage() const { return m_ownerAppStage; }
	
	// System Models
	inline RmlModel_MarkerObjectSystem* getMarkerSystemModel() const { return m_markerSystemModel; }

	// Component Models
	inline RmlModel_AnchorComponent* getAnchorModel() const { return m_anchorModel; }
	inline RmlModel_BoxStencilComponent* getBoxStencilModel() const { return m_boxStencilModel; }
	inline RmlModel_CameraComponent* getCameraModel() const { return m_cameraModel; }
	inline RmlModel_ClientTextureSourceComponent* getClientTextureSourceModel() const { return m_clientTextureSourceModel; }
	inline RmlModel_CompositorComponent* getCompositorModel() const { return m_compositorModel; }
	inline RmlModel_MarkerComponent* getMarkerModel() const { return m_markerModel; }
	inline RmlModel_MarkerTrackingVolumeComponent* getMarkerTrackingVolumeModel() const { return m_markerTrackingVolumeModel; }
	inline RmlModel_ModelStencilComponent* getModelStencilModel() const { return m_modelStencilModel; }
	inline RmlModel_NetworkVideoSourceComponent* getNetworkVideoSourceModel() const { return m_networkVideoSourceModel; }
	inline RmlModel_QuadStencilComponent* getQuadStencilModel() const { return m_quadStencilModel; }
	inline RmlModel_SceneComponent* getSceneModel() const { return m_sceneModel; }
	inline RmlModel_SpoutTextureSourceComponent* getSpoutTextureSourceModel() const { return m_spoutTextureSourceModel; }
	inline RmlModel_StageComponent* getStageModel() const { return m_stageModel; }
	inline RmlModel_TrackingMountComponent* getTrackingMountModel() const { return m_trackingMountModel; }
	inline RmlModel_USBVideoSourceComponent* getUSBVideoSourceModel() const { return m_usbVideoSourceModel; }
	inline RmlModel_VRTrackingVolumeComponent* getVRTrackingVolumeModel() const { return m_vrTrackingVolumeModel; }

private:
	class AppStage_Project* m_ownerAppStage = nullptr;

	// System Models
	RmlModel_MarkerObjectSystem* m_markerSystemModel = nullptr;

	// Component Models
	RmlModel_AnchorComponent* m_anchorModel = nullptr;
	RmlModel_BoxStencilComponent* m_boxStencilModel = nullptr;
	RmlModel_CameraComponent* m_cameraModel = nullptr;
	RmlModel_ClientTextureSourceComponent* m_clientTextureSourceModel = nullptr;
	RmlModel_CompositorComponent* m_compositorModel = nullptr;
	RmlModel_MarkerComponent* m_markerModel = nullptr;
	RmlModel_MarkerTrackingVolumeComponent* m_markerTrackingVolumeModel = nullptr;
	RmlModel_ModelStencilComponent* m_modelStencilModel = nullptr;
	RmlModel_NetworkVideoSourceComponent* m_networkVideoSourceModel = nullptr;
	RmlModel_QuadStencilComponent* m_quadStencilModel = nullptr;
	RmlModel_SceneComponent* m_sceneModel = nullptr;
	RmlModel_SpoutTextureSourceComponent* m_spoutTextureSourceModel = nullptr;
	RmlModel_StageComponent* m_stageModel = nullptr;
	RmlModel_TrackingMountComponent* m_trackingMountModel = nullptr;
	RmlModel_USBVideoSourceComponent* m_usbVideoSourceModel = nullptr;
	RmlModel_VRTrackingVolumeComponent* m_vrTrackingVolumeModel = nullptr;
};

