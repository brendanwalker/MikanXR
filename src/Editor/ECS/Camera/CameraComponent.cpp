#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CameraMath.h"
#include "App.h"
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MikanVideoSourceTypes.h"
#include "MainWindow.h"
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
#include "TrackingSystemsConfig.h"
#include "VideoSourceSystem.h"
#include "VideoSourceComponent.h"
#include "VRDeviceComponent.h"
#include "VRObjectSystem.h"

// -- CameraConfig -----
const std::string CameraDefinition::k_ownerStageIdPropertyId = "stage_id";
const std::string CameraDefinition::k_trackingMountIdPropertyId = "tracking_mount_id";
const std::string CameraDefinition::k_videoSourceIdPropertyId = "video_source_id";
const std::string CameraDefinition::k_trackingFrameDelayPropertyId = "tracking_frame_delay";
const std::string CameraDefinition::k_apertureOrientationOffsetPropertyId = "aperture_orientation_offset";
const std::string CameraDefinition::k_aperturePositionOffsetPropertyId = "aperture_position_offset";

CameraDefinition::CameraDefinition()
	: TransformComponentDefinition()
	, m_cameraId(INVALID_MIKAN_ID)
	, m_stageId(INVALID_MIKAN_ID)
	, m_trackingMountId(INVALID_MIKAN_ID)
	, m_videoSourceId(INVALID_MIKAN_ID)
	, m_trackingFrameDelay(0)
{
}

CameraDefinition::CameraDefinition(
	const std::string& cameraName,
	const struct MikanTransform& xform,
	MikanCameraID cameraId,
	MikanStageID stageId)
	: TransformComponentDefinition(cameraName, xform)
	, m_cameraId(cameraId)
	, m_stageId(stageId)
	, m_trackingMountId(INVALID_MIKAN_ID)
	, m_videoSourceId(INVALID_MIKAN_ID)
	, m_trackingFrameDelay(0)
{
}

configuru::Config CameraDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["id"] = m_cameraId;
	pt["stage_id"] = m_stageId;
	pt["tracking_mount_id"] = m_trackingMountId;
	pt["video_source_id"] = m_videoSourceId;
	pt["tracking_frame_delay"] = m_trackingFrameDelay;

	writeQuaderntiond(pt, "aperture_orientation_offset", m_apertureOrientationOffset);
	writeVector3d(pt, "aperture_position_offset", m_aperturePositionOffset);

	return pt;
}

void CameraDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_cameraId = pt.get<int>("id");
	m_stageId = pt.get_or<int>("stage_id", m_stageId);
	m_trackingMountId = pt.get_or<int>("tracking_mount_id", m_trackingMountId);
	m_videoSourceId = pt.get_or<int>("video_source_id", m_videoSourceId);
	m_trackingFrameDelay = pt.get_or<int>("tracking_frame_delay", m_trackingFrameDelay);

	readQuaterniond(pt, "aperture_orientation_offset", m_apertureOrientationOffset);
	readVector3d(pt, "aperture_position_offset", m_aperturePositionOffset);
}

MikanCameraInfo CameraDefinition::getCameraInfo() const
{
	const std::string& cameraName = getComponentName();
	MikanTransform relativeXform = glm_transform_to_MikanTransform(getRelativeTransform());

	MikanCameraInfo cameraInfo = {};
	cameraInfo.camera_id = m_cameraId;
	cameraInfo.camera_name = cameraName;
	cameraInfo.stage_id = m_stageId;
	cameraInfo.tracking_mount_id = m_trackingMountId;
	cameraInfo.video_source_id = m_videoSourceId;
	cameraInfo.tracking_frame_delay = m_trackingFrameDelay;

	return cameraInfo;
}

void CameraDefinition::setOwnerStageId(MikanStageID stageId)
{
	if (stageId != m_stageId)
	{
		m_stageId = stageId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_ownerStageIdPropertyId));
	}
}

void CameraDefinition::setTrackingMountId(MikanTrackingMountID trackingMountId)
{
	if (trackingMountId != m_trackingMountId)
	{
		m_trackingMountId = trackingMountId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountIdPropertyId));
	}
}

void CameraDefinition::setVideoSourceId(MikanVideoSourceID videoSourceId)
{
	if (videoSourceId != m_videoSourceId)
	{
		m_videoSourceId = videoSourceId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_videoSourceIdPropertyId));
	}
}

void CameraDefinition::setTrackingFrameDelay(int trackingFrameDelay)
{
	if (trackingFrameDelay != m_trackingFrameDelay)
	{
		m_trackingFrameDelay = trackingFrameDelay;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingFrameDelayPropertyId));
	}
}

void CameraDefinition::setAperturePoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
	m_apertureOrientationOffset = q;
	m_aperturePositionOffset = p;
	markDirty(ConfigPropertyChangeSet()
		.addPropertyName(k_apertureOrientationOffsetPropertyId)
		.addPropertyName(k_aperturePositionOffsetPropertyId));
}

// -- CameraComponent -----
const std::string CameraComponent::k_alignCameraFunctionId = "align_camera";
const std::string CameraComponent::k_deleteCameraFunctionId = "delete_camera";

CameraComponent::CameraComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsCustomRender= true;
	m_bWantsUpdate = true;
}

void CameraComponent::init()
{
	TransformComponent::init();
	
	// Watch selection changes
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Refresh the pose view for the tracking mount
	refreshTrackingMount();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	// Listen for changes to the tracking mount definition
	getCameraDefinition()->OnMarkedDirty += MakeDelegate(this, &CameraComponent::onDefinitionChanged);
}

void CameraComponent::dispose()
{
	getCameraDefinition()->OnMarkedDirty -= MakeDelegate(this, &CameraComponent::onDefinitionChanged);

	TransformComponent::dispose();
}

void CameraComponent::update(float deltaSeconds)
{
	TransformComponent::update(deltaSeconds);

	// Scene-space pose view is used to update the component transform
	// of the camera in the scene
	glm::mat4 poseInStageSpace;
	if (m_trackingMountPoseView_SceneSpace && 
		m_trackingMountPoseView_SceneSpace->getPose(
			getSelfPtr<CameraComponent>(),
			poseInStageSpace))
	{
		setRelativeTransform(GlmTransform(poseInStageSpace));
	}
}

void CameraComponent::customRender()
{	
	TextStyle style = getDefaultTextStyle();

	CameraDefinitionPtr CameraDefinition= getCameraDefinition();
	wchar_t wszCameraName[256];
	StringUtils::convertMbsToWcs(CameraDefinition->getComponentName().c_str(), wszCameraName, sizeof(wszCameraName));
	glm::mat4 CameraXform = getWorldTransform();
	glm::vec3 CameraPos(CameraXform[3]);

	glm::vec3 xColor = Colors::DarkRed;
	glm::vec3 yColor = Colors::DarkGreen;
	glm::vec3 zColor = Colors::DarkBlue;
	SelectionComponentPtr selectionComponent = m_selectionComponent.lock();
	if (selectionComponent)
	{
		if (selectionComponent->getIsSelected())
		{
			xColor = Colors::Red;
			yColor = Colors::Green;
			zColor = Colors::Blue;
		}
		else if (selectionComponent->getIsHovered())
		{
			xColor = Colors::LightGreen;
			yColor = Colors::LightGreen;
			zColor = Colors::LightBlue;
		}
	}

	drawTransformedAxes(CameraXform, 0.1f, 0.1f, 0.1f, xColor, yColor, zColor);
	drawTextAtWorldPosition(style, CameraPos, L"%s", wszCameraName);
}

StageComponentConstPtr CameraComponent::getOwnerStageComponent() const
{
	MikanStageID stageId = getCameraDefinition()->getOwnerStageId();

	return StageObjectSystem::getSystem()->getStageById(stageId);
}

VRTrackingSystemDefinitionConstPtr CameraComponent::getVRTrackingSystemDefinition() const
{
	StageComponentConstPtr ownerStage = getOwnerStageComponent();
	if (ownerStage != nullptr)
	{
		TrackingSystemDefinitionConstPtr trackingSystem = ownerStage->getTrackingSystemDefinitionConst();
		if (trackingSystem != nullptr &&
			trackingSystem->getTrackingSystemType() == eTrackingSystemType::vr)
		{
			return std::static_pointer_cast<const VRTrackingSystemDefinition>(trackingSystem);
		}
	}

	return VRTrackingSystemDefinitionConstPtr();
}

TrackingMountDefinitionConstPtr CameraComponent::getTrackingMountDefinition() const
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();
	MikanTrackingMountID trackingMountId = cameraDefinition->getTrackingMountId();

	if (trackingMountId != INVALID_MIKAN_ID)
	{
		VRTrackingSystemDefinitionConstPtr vrTrackingSystem = getVRTrackingSystemDefinition();

		if (vrTrackingSystem)
		{
			return vrTrackingSystem->getTrackingMountDefinitionConst(trackingMountId);
		}
	}

	return TrackingMountDefinitionConstPtr();
}

TrackingMountDefinitionPtr CameraComponent::getTrackingMountDefinitionMutable()
{
	return std::const_pointer_cast<TrackingMountDefinition>(getTrackingMountDefinition());
}

VideoSourceComponentPtr CameraComponent::getVideoSourceComponent() const
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();
	MikanVideoSourceID videoSourceId = cameraDefinition->getVideoSourceId();
	if (videoSourceId != INVALID_MIKAN_ID)
	{
		return VideoSourceSystem::getSystem()->getVideoSourceById(videoSourceId);
	}

	return VideoSourceComponentPtr();
}

void CameraComponent::setVideoSourceById(MikanVideoSourceID videoSourceId)
{
	CameraDefinitionPtr cameraDefinition = getCameraDefinition();

	cameraDefinition->setVideoSourceId(videoSourceId);
}

bool CameraComponent::getAperturePixelDimensions(int& outWidth, int& outHeight) const
{
	VideoSourceComponentPtr videoSourceComponent = getVideoSourceComponent();
	if (videoSourceComponent)
	{
		return videoSourceComponent->getPixelDimensions(outWidth, outHeight);
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

bool CameraComponent::getAperturePose(
	glm::mat4& outCameraPose,
	eVRDevicePoseSpace space) const
{
	// Get the pose of the VR device we want to compute the camera pose from
	bool bValidPose = false;
	glm::mat4 vrDevicePose;
	switch (space)
	{
	case eVRDevicePoseSpace::MikanScene:
		bValidPose = m_trackingMountPoseView_SceneSpace->getPose(
			getSelfPtr<const CameraComponent>(),
			vrDevicePose);
		break;
	case eVRDevicePoseSpace::VRTrackingSystem:
		bValidPose = m_trackingMountPoseView_VRSpace->getPose(
			getSelfPtr<const CameraComponent>(),
			vrDevicePose);
		break;
	}

	// Apply the pre-calibrated offset from the VR device to the camera aperture
	if (bValidPose)
	{
		// Get the offset from the puck to the camera
		CameraDefinitionPtr cameraDefinition= getCameraDefinition();
		const glm::vec3 cameraOffsetPos = 
			MikanVector3d_to_glm_dvec3(cameraDefinition->getApertureOffsetPosition());
		const glm::quat cameraOffsetQuat = 
			MikanQuatd_to_glm_dquat(cameraDefinition->getApertureOffsetOrientation());
		const glm::mat4 cameraOffsetXform = glm_mat4_from_pose(cameraOffsetQuat, cameraOffsetPos);

		// Update the transform of the camera so that vr models align over the tracking puck
		outCameraPose = glm_composite_xform(cameraOffsetXform, vrDevicePose);

		return true;
	}

	return false;
}

bool CameraComponent::getAperturePose(
	glm::dmat4& outCameraPose,
	eVRDevicePoseSpace space) const
{
	glm::mat4 cameraPose;
	if (getAperturePose(cameraPose, space))
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
	if (getAperturePose(cameraPose))
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

void CameraComponent::onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(CameraDefinition::k_trackingMountIdPropertyId))
	{
		refreshTrackingMount();
	}
}

void CameraComponent::refreshTrackingMount()
{
	// Forget the old pose views
	m_trackingMountPoseView_SceneSpace = nullptr;
	m_trackingMountPoseView_VRSpace = nullptr;

	// Try and create a new pose views for the tracking mount
	TrackingMountDefinitionConstPtr trackingMount = getTrackingMountDefinition();
	if (trackingMount)
	{
		VRDeviceComponentPtr vrDeviceComponent =
			VRObjectSystem::getSystem()->getVRDeviceByPath(trackingMount->getDevicePath());

		if (vrDeviceComponent)
		{
			// Tracking mount pose in the space of the stage the camera is in
			m_trackingMountPoseView_SceneSpace = 
				vrDeviceComponent->makePoseView(
					eVRDevicePoseSpace::MikanScene,
					trackingMount->getSocketName());

			// Tracking mount pose in the space of the VR tracking system
			m_trackingMountPoseView_VRSpace =
				vrDeviceComponent->makePoseView(
					eVRDevicePoseSpace::VRTrackingSystem,
					trackingMount->getSocketName());
		}
	}
}

void CameraComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_alignCameraFunctionId);
	outPropertyNames.push_back(k_deleteCameraFunctionId);
}

bool CameraComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (TransformComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == CameraComponent::k_alignCameraFunctionId)
	{
		outDescriptor = {CameraComponent::k_alignCameraFunctionId, "Align Camera"};
		return true;
	}
	else if (functionName == CameraComponent::k_deleteCameraFunctionId)
	{
		outDescriptor = {CameraComponent::k_deleteCameraFunctionId, "Delete Camera"};
		return true;
	}

	return false;
}

bool CameraComponent::invokeFunction(const std::string& functionName)
{
	if (TransformComponent::invokeFunction(functionName))
		return true;

	if (functionName == CameraComponent::k_alignCameraFunctionId)
	{
		alignCamera();
	}
	else if (functionName == CameraComponent::k_deleteCameraFunctionId)
	{
		deleteCamera();
	}

	return false;
}

void CameraComponent::alignCamera()
{
	auto* mainWindow = MainWindow::getInstance();
	TrackingMountDefinitionConstPtr vrTrackingMount= getTrackingMountDefinition();

	if (vrTrackingMount)
	{
		auto* alignmentCalibration = mainWindow->pushAppStage<AppStage_AlignmentCalibration>();
		alignmentCalibration->setTargetCameraComponent(getSelfPtr<CameraComponent>());
	}
	else
	{
		//TODO
		//// Show Camera Triangulation Tool
		//AppStage_CameraTriangulation* CameraTriangulation = MainWindow::getInstance()->pushAppStage<AppStage_CameraTriangulation>();
		//CameraTriangulation->setTargetCameraDefinition(getCameraDefinition());
	}
}

void CameraComponent::deleteCamera()
{
	getOwnerObject()->deleteSelfConfig();
}