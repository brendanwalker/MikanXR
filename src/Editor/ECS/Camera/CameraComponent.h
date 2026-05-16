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
	CameraDefinition(MikanCameraID cameraId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual bool readFromInitParams(
		MikanObjectSystem* ownerObjectSystemDefinition,
		const Serialization::PolymorphicObjectPtr& initParams) override;

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
	void clearAperturePoseOffset();

	static const std::string k_hasValidApertureOffsetPropertyId;
	bool hasValidApertureOffset() const { return m_bHasValidApertureOffset; }

	// Don't send property update for transform property updates as they are spammy
	virtual bool isAutoNotifyTransformPropertyChangeDisabled() override { return true; }

private:
	MikanStageID m_stageId = INVALID_MIKAN_ID;
	MikanTrackingMountID m_trackingMountId = INVALID_MIKAN_ID;
	MikanVideoSourceID m_videoSourceId = INVALID_MIKAN_ID;
	int m_trackingFrameDelay= 0;
	MikanQuatd m_apertureOrientationOffset;
	MikanVector3d m_aperturePositionOffset;
	bool m_bHasValidApertureOffset = false;
};

class CameraComponent : public TransformComponent
{
public:
	CameraComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;
	virtual void customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const override;

	inline static const std::string k_componentClassName = "CameraComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline CameraDefinitionPtr getCameraDefinition() const
	{
		return std::static_pointer_cast<CameraDefinition>(m_definition);
	}
	inline MikanCameraID getCameraId() const { return getCameraDefinition()->getComponentId(); }
	StageComponentConstPtr getOwnerStageComponent() const;
	eTrackingVolumeType getTrackingVolumeType() const;
	VRTrackingVolumeComponentConstPtr getVRTrackingVolumeComponent() const;
	VRTrackingVolumeDefinitionConstPtr getVRTrackingVolumeDefinition() const;
	VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinitionMutable();
	bool hasValidTrackingMountComponent() const;
	TrackingMountComponentConstPtr getTrackingMountComponent() const;
	TrackingMountDefinitionConstPtr getTrackingMountDefinition() const;
	TrackingMountDefinitionPtr getTrackingMountDefinitionMutable();
	VRDevicePoseViewPtr makeTrackingMountPoseView(eVRDevicePoseSpace space) const;
	VideoSourceComponentPtr getVideoSourceComponent() const;
	void setVideoSourceById(MikanVideoSourceID videoSourceId);

	// Helper functions used to fetch camera lens pose properties
	bool hasValidTrackingMountPoseView() const;
	bool getAperturePixelDimensions(int& outWidth, int& outHeight) const;
	bool areApertureIntrinsicsValid() const;
	bool getApertureIntrinsics(struct MikanVideoSourceIntrinsics& outIntrinsics) const;
	bool hasValidApertureOffsetXform() const;
	bool getApertureOffsetXform(glm::mat4& outAperatureToTrackingMountXform) const;
	bool getStageSpaceAperturePose(glm::mat4& outCameraPose) const;
	bool getStageSpaceAperturePose(glm::dmat4& outCameraPose) const;
	bool getApertureProjectionMatrix(glm::mat4& outProjectionMatrix, bool bVerticalFlip = false) const;
	bool getApertureViewMatrix(glm::mat4& outViewMatrix) const;
	bool getApertureViewProjectionMatrix(glm::mat4& outVPMatrix, bool bVerticalFlip =false) const;

	// Helper function to populate a new frame event with the current camera properties
	bool makeNewCameraFrameEvent(
		int64_t frameIndex, 
		int defaultWidth, int defaultHeight,
		struct MikanCameraNewFrameEvent& newFrameEvent) const;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;
	
	// -- IFunctionInterface ----
	static const std::string k_alignCameraFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(const std::string& functionName) override;

	void alignCamera();

protected:
	void onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);
	void rebuildStageSpacePoseView();
	void onActiveDeviceListChanged(eTrackingRuntime runtime);

private:
	SelectionComponentWeakPtr m_selectionComponent;
	VRDevicePoseViewPtr m_trackingMountPoseView_StageSpace;
};