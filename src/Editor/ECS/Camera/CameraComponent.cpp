#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CameraMath.h"
#include "App.h"
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "AlignCameraByUtilityMarker/AppStage_AlignCameraByUtilityMarker.h"
#include "AlignCameraByOriginMarker/AppStage_AlignCameraByOriginMarker.h"
#include "ModalMessageBox/ModalDialog_MessageBox.h"
#include "Colors.h"
#include "IEditorWindow.h"
#include "MikanCameraEvents.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanVideoSourceTypes.h"
#include "MathGLM.h"
#include "ProjectConfig.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MikanCameraTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"
#include "StageObjectSystem.h"
#include "StageComponent.h"
#include "TrackingVolumeComponent.h"
#include "TrackingMountObjectSystem.h"
#include "VideoSourceComponent.h"
#include "VRDeviceComponent.h"
#include "VRObjectSystem.h"
#include "VideoSourceQueries.h"
#include "VRTrackingVolumeComponent.h"

#include "LuaMath.h"
#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- CameraConfig -----
const std::string CameraDefinition::k_ownerStageIdPropertyId = "stage_id";
const std::string CameraDefinition::k_trackingMountIdPropertyId = "tracking_mount_id";
const std::string CameraDefinition::k_videoSourceIdPropertyId = "video_source_id";
const std::string CameraDefinition::k_trackingFrameDelayPropertyId = "tracking_frame_delay";
const std::string CameraDefinition::k_apertureOrientationOffsetPropertyId = "aperture_orientation_offset";
const std::string CameraDefinition::k_aperturePositionOffsetPropertyId = "aperture_position_offset";
const std::string CameraDefinition::k_hasValidApertureOffsetPropertyId = "has_valid_aperture_offset";

CameraDefinition::CameraDefinition()
	: TransformComponentDefinition()
	, m_stageId(INVALID_MIKAN_ID)
	, m_trackingMountId(INVALID_MIKAN_ID)
	, m_videoSourceId(INVALID_MIKAN_ID)
	, m_trackingFrameDelay(0)
{
	m_apertureOrientationOffset = MikanQuatd{ 1, 0, 0, 0 };
	m_aperturePositionOffset = MikanVector3d{ 0, 0, 0 };
}

CameraDefinition::CameraDefinition(
	MikanCameraID cameraId)
	: TransformComponentDefinition(cameraId)
	, m_stageId(INVALID_MIKAN_ID)
	, m_trackingMountId(INVALID_MIKAN_ID)
	, m_videoSourceId(INVALID_MIKAN_ID)
	, m_trackingFrameDelay(0)
{
}

configuru::Config CameraDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["stage_id"] = m_stageId;
	pt["tracking_mount_id"] = m_trackingMountId;
	pt["video_source_id"] = m_videoSourceId;
	pt["tracking_frame_delay"] = m_trackingFrameDelay;

	writeQuaderntiond(pt, "aperture_orientation_offset", m_apertureOrientationOffset);
	writeVector3d(pt, "aperture_position_offset", m_aperturePositionOffset);
	pt["has_valid_aperture_offset"] = m_bHasValidApertureOffset;

	return pt;
}

void CameraDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_stageId = pt.get_or<int>("stage_id", m_stageId);
	m_trackingMountId = pt.get_or<int>("tracking_mount_id", m_trackingMountId);
	m_videoSourceId = pt.get_or<int>("video_source_id", m_videoSourceId);
	m_trackingFrameDelay = pt.get_or<int>("tracking_frame_delay", m_trackingFrameDelay);

	readQuaterniond(pt, "aperture_orientation_offset", m_apertureOrientationOffset);
	readVector3d(pt, "aperture_position_offset", m_aperturePositionOffset);
	m_bHasValidApertureOffset = 
		pt.get_or<bool>("has_valid_aperture_offset", m_bHasValidApertureOffset);
}

bool CameraDefinition::readFromInitParams(
	MikanObjectSystem* ownerObjectSystem,
	const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TransformComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues = initParams.getTypedPointer<MikanCameraComponentValues>();
	if (componentValues)
	{
		m_stageId = componentValues->stage_id;
		m_trackingMountId = componentValues->tracking_mount_id;
		m_videoSourceId = componentValues->video_source_id;
		m_trackingFrameDelay = componentValues->tracking_frame_delay;
		m_apertureOrientationOffset = componentValues->aperture_orientation_offset;
		m_aperturePositionOffset = componentValues->aperture_position_offset;

		// Make sure our parent is always the stage component (if a stage was given)
		if (m_stageId != INVALID_MIKAN_ID)
		{
			m_parentTransformId = m_stageId;
		}
	}

	if (m_stageId == INVALID_MIKAN_ID)
	{
		// If no owning stage was specified, use the first one
		auto stageSystem = ownerObjectSystem->getObjectSystemOfType<StageObjectSystem>();

		m_stageId = stageSystem->getFirstComponentId();
	}

	return true;
}

void CameraDefinition::setOwnerStageId(MikanStageID stageId)
{
	if (stageId != m_stageId)
	{
		m_stageId = stageId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_ownerStageIdPropertyId));
	}
}

void CameraDefinition::setTrackingMountId(MikanTrackingMountID trackingMountId)
{
	if (trackingMountId != m_trackingMountId)
	{
		m_trackingMountId = trackingMountId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_trackingMountIdPropertyId));
	}
}

void CameraDefinition::setVideoSourceId(MikanVideoSourceID videoSourceId)
{
	if (videoSourceId != m_videoSourceId)
	{
		m_videoSourceId = videoSourceId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_videoSourceIdPropertyId));
	}
}

void CameraDefinition::setTrackingFrameDelay(int trackingFrameDelay)
{
	if (trackingFrameDelay != m_trackingFrameDelay)
	{
		m_trackingFrameDelay = trackingFrameDelay;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_trackingFrameDelayPropertyId));
	}
}

void CameraDefinition::setAperturePoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
	m_apertureOrientationOffset = q;
	m_aperturePositionOffset = p;
	m_bHasValidApertureOffset = true;
	notifyPropertyChanged(ConfigPropertyChangeSet()
		.addPropertyName(k_apertureOrientationOffsetPropertyId)
		.addPropertyName(k_aperturePositionOffsetPropertyId)
		.addPropertyName(k_hasValidApertureOffsetPropertyId));
}

void CameraDefinition::clearAperturePoseOffset()
{
	if (m_bHasValidApertureOffset)
	{
		m_apertureOrientationOffset = MikanQuatd{ 1, 0, 0, 0 };
		m_aperturePositionOffset = MikanVector3d{ 0, 0, 0 };
		m_bHasValidApertureOffset = false;
		notifyPropertyChanged(ConfigPropertyChangeSet()
			.addPropertyName(k_apertureOrientationOffsetPropertyId)
			.addPropertyName(k_aperturePositionOffsetPropertyId)
			.addPropertyName(k_hasValidApertureOffsetPropertyId));
	}
}

// -- CameraComponent -----
const std::string CameraComponent::k_alignCameraFunctionId = "align_camera";

CameraComponent::CameraComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsUpdate = true;
}

// -- IEntityAccessor ----
rfk::Struct const* CameraComponent::getClientAPIValuesStructType() const
{
	return &MikanCameraComponentValues::staticGetArchetype();
}

void CameraComponent::init()
{
	TransformComponent::init();
	
	// Watch selection changes
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Refresh the pose view for the tracking mount
	rebuildStageSpacePoseView();

	// Push our world transform to all child stageView components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	// Listen for changes to the tracking mount definition
	getCameraDefinition()->OnPropertyChanged += MakeDelegate(this, &CameraComponent::onDefinitionChanged);

	// Re-refresh when VR devices connect/disconnect
	auto vrObjectSystem = getObjectSystemOfType<VRObjectSystem>();
	vrObjectSystem->OnActiveDeviceListChanged += MakeDelegate(this, &CameraComponent::onActiveDeviceListChanged);
}

void CameraComponent::dispose()
{
	auto vrObjectSystem = getObjectSystemOfType<VRObjectSystem>();
	vrObjectSystem->OnActiveDeviceListChanged -= MakeDelegate(this, &CameraComponent::onActiveDeviceListChanged);

	getCameraDefinition()->OnPropertyChanged -= MakeDelegate(this, &CameraComponent::onDefinitionChanged);

	TransformComponent::dispose();
}

void CameraComponent::update(float deltaSeconds)
{
	TransformComponent::update(deltaSeconds);

	// If the camera is attached to a tracking puck, update the transform of the camera aperture
	if (hasValidTrackingMountComponent())
	{
		updateAperturePoseFromTrackingMount();
	}
}

void CameraComponent::updateAperturePoseFromTrackingMount()
{
	assert(hasValidTrackingMountComponent());

	glm::mat4 poseInStageSpace;
	if (getStageSpaceAperturePose(poseInStageSpace))
	{
		setRelativeTransform(GlmTransform(poseInStageSpace));
	}
}

void CameraComponent::customRender(
	IMkGraphicsContext* graphicsContext, 
	MikanCameraPtr viewportCamera) const
{	
	CameraDefinitionPtr cameraDefinition= getCameraDefinition();
	const glm::mat4 glmCameraXform = getRelativeTransform().getMat4();
	glm::vec3 cameraPos(glmCameraXform[3]);

	// Draw the camera name at the camera position
	TextStyle style = getDefaultTextStyle();
	wchar_t wszCameraName[256];
	StringUtils::convertMbsToWcs(cameraDefinition->getComponentName().c_str(), wszCameraName, sizeof(wszCameraName));
	drawTextAtWorldPosition(graphicsContext, style, cameraPos, L"%s", wszCameraName);

	// Render the camera frustum if the camera has calibrated intrinsics
	MikanVideoSourceIntrinsics intrinsics;
	if (getApertureIntrinsics(intrinsics))
	{
		const auto monoIntrinsics = intrinsics.getMonoIntrinsics();

		// Draw the frustum for the camera
		const float hfov_radians = degrees_to_radians(monoIntrinsics.hfov);
		const float vfov_radians = degrees_to_radians(monoIntrinsics.vfov);
		const float zNear = fmaxf(monoIntrinsics.znear, 0.1f);
		const float zFar = fminf(monoIntrinsics.zfar, 2.0f);

		drawTransformedFrustum(
			graphicsContext,
			glmCameraXform,
			hfov_radians, vfov_radians,
			zNear, zFar,
			Colors::Yellow);
	}

	// Draw the camera transform
	drawTransformedAxes(graphicsContext, glmCameraXform, 0.1f);
}

StageComponentConstPtr CameraComponent::getOwnerStageComponent() const
{
	MikanStageID stageId = getCameraDefinition()->getOwnerStageId();

	return getObjectSystemOfType<StageObjectSystem>()->getStageById(stageId);
}

eTrackingVolumeType CameraComponent::getTrackingVolumeType() const
{
	StageComponentConstPtr ownerStage = getOwnerStageComponent();
	if (ownerStage != nullptr)
	{
		TrackingVolumeDefinitionConstPtr trackingVolume = ownerStage->getTrackingVolumeDefinitionConst();
		if (trackingVolume != nullptr)
		{
			return trackingVolume->getTrackingVolumeType();
		}
	}
	return eTrackingVolumeType::INVALID;
}

VRTrackingVolumeComponentConstPtr CameraComponent::getVRTrackingVolumeComponent() const
{
	StageComponentConstPtr ownerStage = getOwnerStageComponent();
	if (ownerStage != nullptr)
	{
		TrackingVolumeComponentConstPtr trackingVolume = ownerStage->getTrackingVolumeConst();
		if (trackingVolume != nullptr &&
			trackingVolume->getTrackingVolumeType() == eTrackingVolumeType::vr)
		{
			return std::static_pointer_cast<const VRTrackingVolumeComponent>(trackingVolume);
		}
	}

	return VRTrackingVolumeComponentConstPtr();
}
VRTrackingVolumeDefinitionConstPtr CameraComponent::getVRTrackingVolumeDefinition() const
{
	VRTrackingVolumeComponentConstPtr vrTrackingVolumeComponent = getVRTrackingVolumeComponent();
	if (vrTrackingVolumeComponent != nullptr)
	{
		return vrTrackingVolumeComponent->getVRTrackingVolumeDefinition();
	}

	return VRTrackingVolumeDefinitionConstPtr();
}

VRTrackingVolumeDefinitionPtr CameraComponent::getVRTrackingVolumeDefinitionMutable() 
{
	return std::const_pointer_cast<VRTrackingVolumeDefinition>(getVRTrackingVolumeDefinition());
}

bool CameraComponent::hasValidTrackingMountComponent() const
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();
	MikanTrackingMountID trackingMountId = cameraDefinition->getTrackingMountId();

	return (trackingMountId != INVALID_MIKAN_ID);
}

TrackingMountComponentConstPtr CameraComponent::getTrackingMountComponent() const
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();
	MikanTrackingMountID trackingMountId = cameraDefinition->getTrackingMountId();
	if (trackingMountId != INVALID_MIKAN_ID)
	{
		auto trackingMountSystem= getObjectSystemOfType<TrackingMountObjectSystem>(); 
		return trackingMountSystem->getTypedComponentById(trackingMountId);
	}
	return TrackingMountComponentConstPtr();
}

TrackingMountDefinitionConstPtr CameraComponent::getTrackingMountDefinition() const
{
	TrackingMountComponentConstPtr trackingMountComponent = getTrackingMountComponent();
	if (trackingMountComponent != nullptr)
	{
		return trackingMountComponent->getTrackingMountDefinition();
	}
	return TrackingMountDefinitionConstPtr();
}

TrackingMountDefinitionPtr CameraComponent::getTrackingMountDefinitionMutable()
{
	return std::const_pointer_cast<TrackingMountDefinition>(getTrackingMountDefinition());
}

VRDevicePoseViewPtr CameraComponent::makeTrackingMountPoseView(eVRDevicePoseSpace space) const
{
	TrackingMountComponentConstPtr trackingMountComponent = getTrackingMountComponent();
	if (trackingMountComponent != nullptr)
	{
		return trackingMountComponent->makePoseView(space);
	}

	return VRDevicePoseViewPtr();
}

VideoSourceComponentPtr CameraComponent::getVideoSourceComponent() const
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();
	MikanVideoSourceID videoSourceId = cameraDefinition->getVideoSourceId();
	if (videoSourceId != INVALID_MIKAN_ID)
	{
		return VideoSourceQueries::getVideoSourceById(getOwnerProjectManager(), videoSourceId);
	}

	return VideoSourceComponentPtr();
}

void CameraComponent::setVideoSourceById(MikanVideoSourceID videoSourceId)
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();

	cameraDefinition->setVideoSourceId(videoSourceId);
}

bool CameraComponent::hasValidTrackingMountPoseView() const
{
	return m_trackingMountPoseView_StageSpace != nullptr;
}

bool CameraComponent::getAperturePixelDimensions(int& outWidth, int& outHeight) const
{
	VideoSourceComponentPtr videoSourceComponent = getVideoSourceComponent();
	if (videoSourceComponent)
	{
		return videoSourceComponent->getVideoPixelDimensions(outWidth, outHeight);
	}

	return false;
}

bool CameraComponent::areApertureIntrinsicsValid() const
{
	VideoSourceComponentPtr videoSourceComponent = getVideoSourceComponent();
	if (videoSourceComponent)
	{
		return videoSourceComponent->areCameraIntrinsicsValid();
	}
	return false;
}

bool CameraComponent::getApertureIntrinsics(MikanVideoSourceIntrinsics& outIntrinsics) const
{
	VideoSourceComponentPtr videoSourceComponent = getVideoSourceComponent();
	if (videoSourceComponent)
	{
		return videoSourceComponent->getCameraIntrinsics(outIntrinsics);
	}

	return false;
}

bool CameraComponent::hasValidApertureOffsetXform() const
{
	return getCameraDefinition()->hasValidApertureOffset();
}

bool CameraComponent::getApertureOffsetXform(glm::mat4& outTrackingMountToApertureXform) const
{
	// Get the offset from the puck to the camera
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();

	if (cameraDefinition->hasValidApertureOffset())
	{
		const glm::vec3 apertureOffsetPos =
			MikanVector3d_to_glm_dvec3(cameraDefinition->getApertureOffsetPosition());
		const glm::quat apertureOffsetQuat =
			MikanQuatd_to_glm_dquat(cameraDefinition->getApertureOffsetOrientation());

		outTrackingMountToApertureXform = 
			glm_mat4_from_pose(apertureOffsetQuat, apertureOffsetPos);

		return true;
	}

	return false;
}

bool CameraComponent::getStageSpaceAperturePose(glm::mat4& outCameraPose) const
{
	// Compute the aperture pose in stage space 
	glm::mat4 trackingMountPose_StageSpace;
	glm::mat4 trackingMountToApertureXform;
	if (m_trackingMountPoseView_StageSpace &&
		m_trackingMountPoseView_StageSpace->getPose(
			getSelfPtr<const CameraComponent>(),
			trackingMountPose_StageSpace) &&
		getApertureOffsetXform(trackingMountToApertureXform))
	{
		outCameraPose = 
			glm_composite_xform(
				trackingMountToApertureXform, trackingMountPose_StageSpace);

		return true;
	}

	return false;
}

bool CameraComponent::getStageSpaceAperturePose(glm::dmat4& outCameraPose) const
{
	glm::mat4 cameraPose;
	if (getStageSpaceAperturePose(cameraPose))
	{
		outCameraPose = glm::dmat4(cameraPose);
		return true;
	}

	return false;
}

bool CameraComponent::getApertureProjectionMatrix(
	glm::mat4& outProjectionMatrix,
	bool bVerticalFlip) const
{
	VideoSourceComponentPtr videoSourceComponent = getVideoSourceComponent();
	if (videoSourceComponent)
	{
		outProjectionMatrix = videoSourceComponent->getProjectionMatrix();

		if (bVerticalFlip)
		{
			// Flip the projection matrix to account for OpenGL's inverted Y-axis
			outProjectionMatrix =
				glm::scale(glm::mat4(1.0), glm::vec3(1.f, -1.f, 1.f)) *
				outProjectionMatrix;
		}

		return true;
	}

	return false;
}

bool CameraComponent::getApertureViewMatrix(glm::mat4& outViewMatrix) const
{
	glm::mat4 cameraPose;
	if (getStageSpaceAperturePose(cameraPose))
	{
		outViewMatrix = computeGLMCameraViewMatrix(cameraPose);
		return true;
	}

	return false;
}

bool CameraComponent::getApertureViewProjectionMatrix(
	glm::mat4& outVPMatrix,
	bool bVerticalFlip) const
{
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;
	if (getApertureProjectionMatrix(projMatrix, bVerticalFlip) && getApertureViewMatrix(viewMatrix))
	{
		outVPMatrix = projMatrix * viewMatrix;
		return true;
	}

	return false;
}

bool CameraComponent::makeNewCameraFrameEvent(
	int64_t frameIndex, 
	int defaultWidth, int defaultHeight,
	MikanCameraNewFrameEvent& outNewFrameEvent) const
{
	outNewFrameEvent = {};
	outNewFrameEvent.camera_id = getCameraId();
	outNewFrameEvent.frame = frameIndex;

	// Assign Camera Extrinsic values
	const glm::mat4 cameraXform = getWorldTransform();
	const glm::vec3 cameraUp(cameraXform[1]); // Camera up is along the y-axis
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]); // Camera up is along the y-axis
	outNewFrameEvent.camera_forward = glm_vec3_to_MikanVector3f(cameraForward);
	outNewFrameEvent.camera_up = glm_vec3_to_MikanVector3f(cameraUp);
	outNewFrameEvent.camera_position = glm_vec3_to_MikanVector3f(cameraPosition);

	// Assign Camera Intrinsic values
	MikanVideoSourceIntrinsics intrinsics = {};

	// Try fetching calibrated camera intrinsics
	if (getApertureIntrinsics(intrinsics))
	{
		if (intrinsics.intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS)
		{
			const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();
			const MikanMatrix3d& cameraMatrix = monoIntrinsics.undistorted_camera_matrix;

			outNewFrameEvent.focal_length = { cameraMatrix.x0, cameraMatrix.y1 };
			outNewFrameEvent.principal_point = { cameraMatrix.x2, cameraMatrix.y2 };
			outNewFrameEvent.pixel_size = { (int)monoIntrinsics.pixel_width, (int)monoIntrinsics.pixel_height };
			outNewFrameEvent.z_bounds = { monoIntrinsics.znear, monoIntrinsics.zfar };

			return true;
		}
		else if (intrinsics.intrinsics_type == MikanIntrinsicsType::STEREO_CAMERA_INTRINSICS)
		{
			//TODO: Assume we are using the left eye's point of view for client compositing
			const MikanStereoIntrinsics& stereoIntrinsics = intrinsics.getStereoIntrinsics();
			const MikanMatrix3d& cameraMatrix = stereoIntrinsics.left_camera_matrix;

			outNewFrameEvent.focal_length = { cameraMatrix.x0, cameraMatrix.y1 };
			outNewFrameEvent.principal_point = { cameraMatrix.x2, cameraMatrix.y2 };
			outNewFrameEvent.pixel_size = { (int)stereoIntrinsics.pixel_width, (int)stereoIntrinsics.pixel_height };
			outNewFrameEvent.z_bounds = { stereoIntrinsics.znear, stereoIntrinsics.zfar };

			return true;
		}
	}
	// Fallback to fake intrinsics if we just have a frame resolution available
	// (focal length with just be some default value that is probably wrong)
	else
	{
		int pixelWidth, pixelHeight;
		if (!getAperturePixelDimensions(pixelWidth, pixelHeight))
		{
			pixelWidth = defaultWidth;
			pixelHeight = defaultHeight;
		}

		if (pixelWidth > 0 && pixelHeight > 0)
		{
			MikanMonoIntrinsics fakeIntrinsics = {};
			createDefautMonoIntrinsics(pixelWidth, pixelHeight, fakeIntrinsics);
			const MikanMatrix3d& cameraMatrix = fakeIntrinsics.undistorted_camera_matrix;

			outNewFrameEvent.focal_length = { cameraMatrix.x0, cameraMatrix.y1 };
			outNewFrameEvent.principal_point = { cameraMatrix.x2, cameraMatrix.y2 };
			outNewFrameEvent.pixel_size = { pixelWidth, pixelHeight };
			outNewFrameEvent.z_bounds = { fakeIntrinsics.znear, fakeIntrinsics.zfar };

			return true;
		}
	}

	return false;
}

void CameraComponent::onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(CameraDefinition::k_trackingMountIdPropertyId))
	{
		rebuildStageSpacePoseView();
	}
}

void CameraComponent::onActiveDeviceListChanged(eTrackingRuntime runtime)
{
	rebuildStageSpacePoseView();
}

void CameraComponent::rebuildStageSpacePoseView()
{
	// Forget the old pose views
	m_trackingMountPoseView_StageSpace = nullptr;

	// Try and create a new pose views for the tracking mount
	TrackingMountDefinitionConstPtr trackingMount = getTrackingMountDefinition();
	if (trackingMount)
	{
		auto vrObjectSystem = getObjectSystemOfType<VRObjectSystem>();
		VRDeviceComponentPtr vrDeviceComponent = 
			vrObjectSystem->getVRDeviceByPath(trackingMount->getDevicePath());

		if (vrDeviceComponent)
		{
			// Tracking mount pose in the space of the stage the camera is in
			m_trackingMountPoseView_StageSpace = 
				vrDeviceComponent->makePoseView(
					eVRDevicePoseSpace::MikanTrackingVolumePose,
					trackingMount->getSocketName());

			// Recompute the aperture pose since the tracking mount pose view has changed
			updateAperturePoseFromTrackingMount();
		}
	}
}

// -- IPropertyInterface ----
void CameraComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_ownerStageIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(-1)
		->setReadOnly()
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_trackingMountIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(-1)
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_videoSourceIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(-1)
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_trackingFrameDelayPropertyId, MikanVariantType::INT)
		->setDefaultValue(0));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_apertureOrientationOffsetPropertyId, MikanVariantType::QUATERNIOND)
		->setDefaultValue(MikanVector3f(0.f, 0.f, 0.f))
		->setReadOnly()
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_aperturePositionOffsetPropertyId, MikanVariantType::VECTOR3F)
		->setDefaultValue(MikanVector3f(0.f, 0.f, 0.f))
		->setReadOnly()
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(CameraDefinition::k_hasValidApertureOffsetPropertyId, MikanVariantType::BOOL)
		->setDefaultValue(false)
		->setReadOnly()
		->setUIHidden());
}

bool CameraComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == CameraDefinition::k_ownerStageIdPropertyId)
	{
		outValue = getCameraDefinition()->getOwnerStageId();
		return true;
	}
	else if (propertyName == CameraDefinition::k_trackingMountIdPropertyId)
	{
		outValue = getCameraDefinition()->getTrackingMountId();
		return true;
	}
	else if (propertyName == CameraDefinition::k_videoSourceIdPropertyId)
	{
		outValue = getCameraDefinition()->getVideoSourceId();
		return true;
	}
	else if (propertyName == CameraDefinition::k_trackingFrameDelayPropertyId)
	{
		outValue = getCameraDefinition()->getTrackingFrameDelay();
		return true;
	}
	else if (propertyName == CameraDefinition::k_apertureOrientationOffsetPropertyId)
	{
		outValue = getCameraDefinition()->getApertureOffsetOrientation();
		return true;
	}
	else if (propertyName == CameraDefinition::k_aperturePositionOffsetPropertyId)
	{
		outValue = getCameraDefinition()->getApertureOffsetPosition();
		return true;
	}
	else if (propertyName == CameraDefinition::k_hasValidApertureOffsetPropertyId)
	{
		outValue = getCameraDefinition()->hasValidApertureOffset();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool CameraComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == CameraDefinition::k_trackingMountIdPropertyId)
	{
		MikanTrackingMountID trackingMountId = static_cast<MikanTrackingMountID>(inValue.getIntValue());
		getCameraDefinition()->setTrackingMountId(trackingMountId);
		return true;
	}
	else if (propertyName == CameraDefinition::k_videoSourceIdPropertyId)
	{
		MikanVideoSourceID videoSourceId = static_cast<MikanVideoSourceID>(inValue.getIntValue());
		getCameraDefinition()->setVideoSourceId(videoSourceId);
		return true;
	}
	else if (propertyName == CameraDefinition::k_trackingFrameDelayPropertyId)
	{
		int trackingFrameDelay = inValue.getIntValue();
		getCameraDefinition()->setTrackingFrameDelay(trackingFrameDelay);
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
void CameraComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_alignCameraFunctionId, "Align Camera"));
}

bool CameraComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == CameraComponent::k_alignCameraFunctionId)
	{
		alignCamera();
	}

	return TransformComponent::invokeFunction(functionName);
}

void CameraComponent::alignCamera()
{
	AppStage* currentAppStage= getOwnerEditorWindow()->getCurrentAppStage();

	switch (getTrackingVolumeType())
	{
	case eTrackingVolumeType::vr:
		if (hasValidTrackingMountComponent())
		{
			AppStage_AlignmentCalibration::tryEnterAlignmentCalibration(
				currentAppStage,
				getSelfPtr<CameraComponent>());
		}
		else
		{
			AppStage_AlignCameraByUtilityMarker::tryEnterCalibration(
				currentAppStage,
				getSelfPtr<CameraComponent>());
		}
		break;
	case eTrackingVolumeType::marker:
		AppStage_AlignCameraByOriginMarker::tryEnterCalibration(
			currentAppStage,
			getSelfPtr<CameraComponent>());
		break;
	default:
		ModalDialog_MessageBox::showMessageBox(
			currentAppStage,
			"Stage missing tracking volume. Please assign a tracking volume to the stage this camera is attached to.");
		break;
	}
}

// -- Lua Binding ----
void CameraComponent::bindLuaFunctions(struct lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<CameraComponent, TransformComponent>(
			CameraComponent::k_componentClassName.c_str())
		.addProperty("trackingFrameDelay",
			[](CameraComponent* c) -> int {
				return c->getCameraDefinition()->getTrackingFrameDelay();
			},
			[](CameraComponent* c, int v) {
				c->getCameraDefinition()->setTrackingFrameDelay(v);
			})
		.addFunction("alignCamera",
			[](CameraComponent* c) {
				c->alignCamera();
			})
		.addProperty("ownerStageId",
			[](CameraComponent* c) -> int {
				return c->getCameraDefinition()->getOwnerStageId();
			})
		.addProperty("trackingMountId",
			[](CameraComponent* c) -> int {
				return c->getCameraDefinition()->getTrackingMountId();
			})
		.addProperty("videoSourceId",
			[](CameraComponent* c) -> int {
				return c->getCameraDefinition()->getVideoSourceId();
			})
		.addProperty("hasValidApertureOffset",
			[](CameraComponent* c) -> bool {
				return c->getCameraDefinition()->hasValidApertureOffset();
			})
		.addProperty("aperturePositionOffset",
			[](CameraComponent* c) -> LuaVec3f {
				MikanVector3d p = c->getCameraDefinition()->getApertureOffsetPosition();
				return LuaVec3f((float)p.x, (float)p.y, (float)p.z);
			})
		.addProperty("apertureOrientationOffset",
			[](CameraComponent* c) -> LuaVec3f {
				MikanQuatd q = c->getCameraDefinition()->getApertureOffsetOrientation();
				return LuaVec3f(glm_quat_to_MikanRotator3f(MikanQuatd_to_glm_quat(q)));
			})
		.addFunction("getOwnerStage",
			[](CameraComponent* c) -> StageComponent* {
				return const_cast<StageComponent*>(c->getOwnerStageComponent().get());
			})
		.endClass();
}
