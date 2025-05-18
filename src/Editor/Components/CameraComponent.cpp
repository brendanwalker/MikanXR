#include "CameraComponent.h"
#include "SceneObjectSystem.h"
#include "App.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "ProjectConfig.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MikanCameraTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

// -- CameraConfig -----
CameraDefinition::CameraDefinition()
	: SceneComponentDefinition()
{
	m_CameraId = INVALID_MIKAN_ID;
}

CameraDefinition::CameraDefinition(
	MikanCameraID cameraId,
	const std::string& cameraName,
	const MikanTransform& xform)
	: SceneComponentDefinition(cameraName, xform)
	, m_CameraId(cameraId)
{
}

configuru::Config CameraDefinition::writeToJSON()
{
	configuru::Config pt = SceneComponentDefinition::writeToJSON();

	pt["id"] = m_CameraId;

	return pt;
}

void CameraDefinition::readFromJSON(const configuru::Config& pt)
{
	SceneComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_CameraId = pt.get<int>("id");
		m_configName = StringUtils::stringify("Camera_", m_CameraId);
	}
}

// -- CameraComponent -----
CameraComponent::CameraComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
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
	SceneComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_editCameraFunctionId);
	outPropertyNames.push_back(k_deleteCameraFunctionId);
}

bool CameraComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (SceneComponent::getFunctionDescriptor(functionName, outDescriptor))
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
	if (SceneComponent::invokeFunction(functionName))
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

void CameraComponent::extractCameraInfoForClientAPI(MikanCameraInfo& outCameraInfo) const
{
	const std::string CameraName = getName();
	const GlmTransform CameraWorldTransform(getWorldTransform());
	const MikanCameraID CameraId = getCameraDefinition()->getCameraId();

	outCameraInfo.camera_id = getCameraDefinition()->getCameraId();
	outCameraInfo.camera_name= CameraName;
	outCameraInfo.world_transform = glm_transform_to_MikanTransform(CameraWorldTransform);
}

void CameraComponent::editCamera()
{
	CameraDefinitionPtr definition= getCameraDefinition();
	MikanCameraID CameraId= definition->getCameraId();
	CameraComponentPtr CameraComponent = SceneObjectSystem::getSystem()->getCameraById(CameraId);
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