#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "IFrameCoupledPoseProvider.h"
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
using VideoFrameDistortionViewPtr= std::shared_ptr<VideoFrameDistortionView>;

class CameraDefinition : public TransformComponentDefinition
{
public:
	CameraDefinition();
	CameraDefinition(MikanCameraID cameraId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual bool readFromInitParams(MikanObjectSystem* ownerObjectSystemDefinition,
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

	static const std::string k_lightEnvironmentIdPropertyId;
	inline MikanLightID getLightEnvironmentId() const { return m_lightEnvionmentId; }
	void setLightEnvironmentId(MikanLightID lightEnvironmentId);

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

	/// Multiplier applied to MoGe-2's metric depth for captures from this
	/// camera. The model's scale head guesses scale from image appearance, so
	/// an unusual lens can be off by an integer factor while the geometry
	/// stays excellent; the factor is a stable property of the camera/lens, so
	/// it is calibrated once against an ArUco marker of known size and reused.
	static const std::string k_depthMeshScaleCorrectionPropertyId;
	inline float getDepthMeshScaleCorrection() const { return m_depthMeshScaleCorrection; }
	void setDepthMeshScaleCorrection(float scaleCorrection);

	virtual bool wantsSaveForPropertyChange(const ConfigPropertyChangeSet& changedPropertySet) const override
	{
		// Don't trigger save when a transform value changed and attached to a tracking mount
		// (since the position is going to change every frame
		if (m_trackingMountId != INVALID_MIKAN_ID
			&& (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativePositionPropertyId)
				|| changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeQuaternionPropertyId)
				|| changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeScalePropertyId)))
		{
			return false;
		}

		return TransformComponentDefinition::wantsSaveForPropertyChange(changedPropertySet);
	}

private:
	MikanStageID m_stageId= INVALID_MIKAN_ID;
	MikanLightID m_lightEnvionmentId= INVALID_MIKAN_ID;
	MikanTrackingMountID m_trackingMountId= INVALID_MIKAN_ID;
	MikanVideoSourceID m_videoSourceId= INVALID_MIKAN_ID;
	int m_trackingFrameDelay= 0;
	float m_depthMeshScaleCorrection= 1.f;
	MikanQuatd m_apertureOrientationOffset;
	MikanVector3d m_aperturePositionOffset;
	bool m_bHasValidApertureOffset= false;
};

class CameraComponent : public TransformComponent
{
public:
	CameraComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;
	virtual void customRender(IMkGraphicsContext* graphicsContext, MikanCameraPtr viewportCamera) const override;

	inline static const std::string k_componentClassName= "CameraComponent";
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

	// Helper functions used to fetch camera lens properties
	bool hasValidTrackingMountPoseView() const;
	bool getAperturePixelDimensions(int& outWidth, int& outHeight) const;
	bool areApertureIntrinsicsValid() const;
	bool getApertureIntrinsics(struct MikanVideoSourceIntrinsics& outIntrinsics) const;
	bool hasValidApertureOffsetXform() const;
	bool getApertureOffsetXform(glm::mat4& outAperatureToTrackingMountXform) const;
	bool getStageSpaceAperturePose(glm::mat4& outCameraPose) const;
	bool getStageSpaceAperturePose(glm::dmat4& outCameraPose) const;
	bool getApertureProjectionMatrix(glm::mat4& outProjectionMatrix, bool bVerticalFlip= false) const;
	bool getApertureViewMatrix(glm::mat4& outViewMatrix) const;
	bool getApertureViewProjectionMatrix(glm::mat4& outVPMatrix, bool bVerticalFlip= false) const;

	// Helper function to populate a new frame event with the current camera properties
	bool makeNewCameraFrameEvent(int64_t frameIndex, int defaultWidth, int defaultHeight,
								 struct MikanCameraNewFrameEvent& newFrameEvent) const;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_alignCameraFunctionId;
	static const std::string k_captureSceneLightingFunctionId;
	static const std::string k_captureDepthMeshFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(const std::string& functionName) override;

	void alignCamera();
	void captureSceneLighting();
	void captureDepthMesh();

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

protected:
	void onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);
	void rebuildStageSpacePoseView();
	void updateAperturePoseFromTrackingMount();
	void onActiveDeviceListChanged(eTrackingRuntime runtime);

	// Frame-coupled pose (ticket E4) - only applies latest intrinsics to the video
	// source if they changed meaningfully (>1% relative fx/fy), avoiding an
	// unnecessary recomputeCameraProjectionMatrix() every single tick when (as
	// expected) ARKit's reported intrinsics are essentially constant frame to frame.
	void maybeUpdateFrameCoupledIntrinsics(VideoSourceComponentPtr videoSourceComponent,
										   const struct MikanVideoSourceIntrinsics& newIntrinsics);

private:
	SelectionComponentWeakPtr m_selectionComponent;
	VRDevicePoseViewPtr m_trackingMountPoseView_StageSpace;
};