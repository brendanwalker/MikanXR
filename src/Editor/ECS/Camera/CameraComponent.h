#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "TransformComponent.h"
#include "MikanTypeFwd.h"
#include "MikanCameraTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"
#include "VRDeviceComponent.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class VideoFrameDistortionView;
using VideoFrameDistortionViewPtr = std::shared_ptr<VideoFrameDistortionView>;

class CameraDefinition : public TransformComponentDefinition
{
public:
	CameraDefinition();
	CameraDefinition(
		const std::string& cameraName,
		const struct MikanTransform& xform,
		MikanCameraID cameraId, 
		MikanStageID stageId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanCameraID getCameraId() const { return m_cameraId; }
	MikanCameraInfo getCameraInfo() const;

	static const std::string k_ownerStageIdPropertyId;
	inline MikanStageID getOwnerStageId() const { return m_stageId; }
	void setOwnerStageId(MikanStageID stageId);

	static const std::string k_trackingMountIdPropertyId;
	inline MikanTrackingMountID getTrackingMountId() const { return m_trackingMountId; }
	void setTrackingMountId(MikanTrackingMountID trackingMountId);

	static const std::string k_videoSourceIdPropertyId;
	inline MikanVideoSourceID getVideoSourceId() const { return m_videoSourceId; }
	void setVideoSourceId(MikanVideoSourceID videoSourceId);

	static const std::string k_trackingFrameDelayPropertyId;
	inline int getTrackingFrameDelay() const { return m_trackingFrameDelay; }
	void setTrackingFrameDelay(int trackingFrameDelay);

	static const std::string k_apertureOrientationOffsetPropertyId;
	static const std::string k_aperturePositionOffsetPropertyId;
	inline MikanQuatd getApertureOffsetOrientation() const { return m_apertureOrientationOffset; }
	inline MikanVector3d getApertureOffsetPosition() const { return m_aperturePositionOffset; }
	void setAperturePoseOffset(const MikanQuatd& q, const MikanVector3d& p);

private:
	MikanCameraID m_cameraId = INVALID_MIKAN_ID;
	MikanStageID m_stageId = INVALID_MIKAN_ID;
	MikanTrackingMountID m_trackingMountId = INVALID_MIKAN_ID;
	MikanVideoSourceID m_videoSourceId = INVALID_MIKAN_ID;
	int m_trackingFrameDelay= 0;
	MikanQuatd m_apertureOrientationOffset;
	MikanVector3d m_aperturePositionOffset;
};

class CameraComponent : public TransformComponent
{
public:
	CameraComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;
	virtual void customRender() override;

	inline static const std::string k_componentClassName = "CameraComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline CameraDefinitionPtr getCameraDefinition() const
	{
		return std::static_pointer_cast<CameraDefinition>(m_definition);
	}
	inline MikanCameraID getCameraId() const { return getCameraDefinition()->getCameraId(); }
	StageComponentConstPtr getOwnerStageComponent() const;
	VRTrackingVolumeDefinitionConstPtr getVRTrackingVolumeDefinition() const;
	VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinitionMutable();
	TrackingMountDefinitionConstPtr getTrackingMountDefinition() const;
	TrackingMountDefinitionPtr getTrackingMountDefinitionMutable();
	VideoSourceComponentPtr getVideoSourceComponent() const;
	void setVideoSourceById(MikanVideoSourceID videoSourceId);

	// Helper functions used to fetch camera lens pose properties
	bool getAperturePixelDimensions(int& outWidth, int& outHeight) const;
	bool getApertureIntrinsics(struct MikanVideoSourceIntrinsics& outIntrinsics) const;
	bool getAperturePose(
		glm::mat4& outCameraPose, 
		eVRDevicePoseSpace space = eVRDevicePoseSpace::MikanTrackingVolumePose) const;
	bool getAperturePose(
		glm::dmat4& outCameraPose, 
		eVRDevicePoseSpace space = eVRDevicePoseSpace::MikanTrackingVolumePose) const;
	bool getApertureProjectionMatrix(glm::mat4& outProjectionMatrix, bool bVerticalFlip = false) const;
	bool getApertureViewMatrix(glm::mat4& outViewMatrix) const;
	bool getApertureViewProjectionMatrix(glm::mat4& outVPMatrix, bool bVerticalFlip =false) const;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;
	
	// -- IRmlFunctionInterface ----
	static const std::string k_alignCameraFunctionId;
	static const std::string k_deleteCameraFunctionId;
	static void getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void alignCamera();
	void deleteCamera();

protected:
	void onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);
	void refreshTrackingMount();

private:
	SelectionComponentWeakPtr m_selectionComponent;
	VRDevicePoseViewPtr m_trackingMountPoseView_SceneSpace;
	VRDevicePoseViewPtr m_trackingMountPoseView_VRSpace;
};