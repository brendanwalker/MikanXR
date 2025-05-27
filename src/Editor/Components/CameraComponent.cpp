#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "App.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "ProjectConfig.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MikanCameraTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

// -- CameraConfig -----
const std::string CameraDefinition::k_attachedVRDevicePropertyId = "attached_vr_device_id";
const std::string CameraDefinition::k_orientationOffsetPropertyId = "orientation_offset";
const std::string CameraDefinition::k_positionOffsetPropertyId = "position_offset";


CameraDefinition::CameraDefinition()
	: TransformComponentDefinition()
	, m_cameraId(INVALID_MIKAN_ID)
	, m_attachedVRDeviceId(INVALID_MIKAN_ID)
	, m_orientationOffset({1.0, 0.0, 0.0, 0.0})
	, m_positionOffset({0.0, 0.0, 0.0})
{
}

CameraDefinition::CameraDefinition(
	MikanCameraID cameraId,
	const std::string& cameraName,
	const MikanTransform& xform)
	: TransformComponentDefinition(cameraName, xform)
	, m_cameraId(cameraId)
	, m_attachedVRDeviceId(INVALID_MIKAN_ID)
	, m_orientationOffset({1.0, 0.0, 0.0, 0.0})
	, m_positionOffset({0.0, 0.0, 0.0})
{
}

configuru::Config CameraDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["id"] = m_cameraId;

	return pt;
}

void CameraDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_cameraId = pt.get<int>("id");
		m_configName = StringUtils::stringify("Camera_", m_cameraId);
	}
}

MikanCameraInfo CameraDefinition::getCameraInfo() const
{
	const std::string& cameraName = getComponentName();
	MikanTransform relativeXform = glm_transform_to_MikanTransform(getRelativeTransform());

	MikanCameraInfo cameraInfo = {};
	cameraInfo.camera_id = m_cameraId;
	cameraInfo.camera_name = cameraName;
	cameraInfo.relative_transform = relativeXform;

	return cameraInfo;
}

void CameraDefinition::setAttachedVRDeviceId(MikanVRDeviceID deviceId)
{
	if (deviceId != m_attachedVRDeviceId)
	{
		m_attachedVRDeviceId= deviceId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_attachedVRDevicePropertyId));
	}
}

void CameraDefinition::setOrientationOffset(MikanQuatd rotation)
{
	if (memcmp(&m_orientationOffset, &rotation, sizeof(MikanQuatd)))
	{
		m_orientationOffset = rotation;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_orientationOffsetPropertyId));
	}
}

void CameraDefinition::setPositionOffset(MikanVector3d translation)
{
	if (memcmp(&m_positionOffset, &translation, sizeof(MikanVector3d)))
	{
		m_positionOffset = translation;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_positionOffsetPropertyId));
	}
}

// -- CameraComponent -----
CameraComponent::CameraComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsCustomRender= true;
}

void CameraComponent::init()
{
	MikanComponent::init();

	// Watch selection changes
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
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

// -- IFunctionInterface ----
const std::string CameraComponent::k_editCameraFunctionId = "edit_camera";
const std::string CameraComponent::k_deleteCameraFunctionId = "delete_camera";

void CameraComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_editCameraFunctionId);
	outPropertyNames.push_back(k_deleteCameraFunctionId);
}

bool CameraComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (TransformComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == CameraComponent::k_editCameraFunctionId)
	{
		outDescriptor = {CameraComponent::k_editCameraFunctionId, "Edit Camera"};
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

	if (functionName == CameraComponent::k_editCameraFunctionId)
	{
		editCamera();
	}
	else if (functionName == CameraComponent::k_deleteCameraFunctionId)
	{
		deleteCamera();
	}

	return false;
}

void CameraComponent::editCamera()
{
	CameraDefinitionPtr definition= getCameraDefinition();
	MikanCameraID CameraId= definition->getCameraId();
	CameraComponentPtr CameraComponent = CameraObjectSystem::getSystem()->getCameraById(CameraId);
	if (CameraComponent != nullptr)
	{
		//TODO
		//// Show Camera Triangulation Tool
		//AppStage_CameraTriangulation* CameraTriangulation = MainWindow::getInstance()->pushAppStage<AppStage_CameraTriangulation>();
		//
		//CameraTriangulatorInfo CameraInfo = {
		//	definition->getCameraId(),
		//	definition->getRelativeTransform(),
		//	definition->getComponentName()
		//};
		//CameraTriangulation->setTargetCamera(CameraInfo);
	}
}

void CameraComponent::deleteCamera()
{
	getOwnerObject()->deleteSelfConfig();
}