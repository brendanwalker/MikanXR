#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "App.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlTextRenderer.h"
#include "MathGLM.h"
#include "ProfileConfig.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

// -- AnchorConfig -----
AnchorDefinition::AnchorDefinition()
	: SceneComponentDefinition()
{
	m_anchorId = INVALID_MIKAN_ID;
}

AnchorDefinition::AnchorDefinition(
	MikanSpatialAnchorID anchorId,
	const std::string& anchorName,
	const MikanTransform& xform)
	: SceneComponentDefinition(StringUtils::stringify("Anchor_", anchorId), xform)
	, m_anchorId(anchorId)
{
}

configuru::Config AnchorDefinition::writeToJSON()
{
	configuru::Config pt = SceneComponentDefinition::writeToJSON();

	pt["id"] = m_anchorId;

	return pt;
}

void AnchorDefinition::readFromJSON(const configuru::Config& pt)
{
	SceneComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_anchorId = pt.get<int>("id");
		m_configName = StringUtils::stringify("Anchor_", m_anchorId);
	}
}

// -- AnchorComponent -----
AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
	m_bWantsCustomRender= true;
}

void AnchorComponent::init()
{
	MikanComponent::init();

	// Watch selection changes
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
}

void AnchorComponent::customRender()
{	
	if (!AnchorObjectSystem::getSystem()->getAnchorSystemConfig()->getRenderAnchorsFlag())
		return;

	TextStyle style = getDefaultTextStyle();

	AnchorDefinitionPtr anchorDefinition= getAnchorDefinition();
	wchar_t wszAnchorName[MAX_MIKAN_ANCHOR_NAME_LEN];
	StringUtils::convertMbsToWcs(anchorDefinition->getComponentName().c_str(), wszAnchorName, sizeof(wszAnchorName));
	glm::mat4 anchorXform = getWorldTransform();
	glm::vec3 anchorPos(anchorXform[3]);

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

	drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f, xColor, yColor, zColor);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
}

// -- IFunctionInterface ----
const std::string AnchorComponent::k_updateOriginAnchorPoseFunctionId = "update_origin_pose";
const std::string AnchorComponent::k_editAnchorFunctionId = "edit_anchor";
const std::string AnchorComponent::k_deleteAnchorFunctionId = "delete_anchor";

void AnchorComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	SceneComponent::getFunctionNames(outPropertyNames);

	AnchorObjectSystemPtr anchorSystemPtr = AnchorObjectSystem::getSystem();
	AnchorComponentPtr originSpatialAnchor = anchorSystemPtr->getOriginSpatialAnchor();
	if (originSpatialAnchor->getAnchorDefinition()->getAnchorId() == getAnchorDefinition()->getAnchorId())
	{
		outPropertyNames.push_back(k_updateOriginAnchorPoseFunctionId);
	}

	outPropertyNames.push_back(k_editAnchorFunctionId);
	outPropertyNames.push_back(k_deleteAnchorFunctionId);
}

bool AnchorComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (SceneComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == AnchorComponent::k_updateOriginAnchorPoseFunctionId)
	{
		outDescriptor = {AnchorComponent::k_updateOriginAnchorPoseFunctionId, "Update Origin Pose"};
		return true;
	}
	else if (functionName == AnchorComponent::k_editAnchorFunctionId)
	{
		outDescriptor = {AnchorComponent::k_editAnchorFunctionId, "Edit Anchor"};
		return true;
	}
	else if (functionName == AnchorComponent::k_deleteAnchorFunctionId)
	{
		outDescriptor = {AnchorComponent::k_deleteAnchorFunctionId, "Delete Anchor"};
		return true;
	}

	return false;
}

bool AnchorComponent::invokeFunction(const std::string& functionName)
{
	if (SceneComponent::invokeFunction(functionName))
		return true;

	if (functionName == AnchorComponent::k_updateOriginAnchorPoseFunctionId)
	{
		updateOriginAnchorPose();
	}
	else if (functionName == AnchorComponent::k_editAnchorFunctionId)
	{
		editAnchor();
	}
	else if (functionName == AnchorComponent::k_deleteAnchorFunctionId)
	{
		deleteAnchor();
	}

	return false;
}

void AnchorComponent::extractAnchorInfoForClientAPI(MikanSpatialAnchorInfo& outAnchorInfo) const
{
	const std::string anchorName = getName();
	const GlmTransform anchorWorldTransform(getWorldTransform());
	const MikanSpatialAnchorID anchorId = getAnchorDefinition()->getAnchorId();

	memset(&outAnchorInfo, 0, sizeof(MikanSpatialAnchorInfo));
	outAnchorInfo.anchor_id = getAnchorDefinition()->getAnchorId();
	strncpy(outAnchorInfo.anchor_name, anchorName.c_str(), sizeof(outAnchorInfo.anchor_name) - 1);
	outAnchorInfo.world_transform = glm_transform_to_MikanTransform(anchorWorldTransform);
}

void AnchorComponent::updateOriginAnchorPose()
{
	ProfileConfigPtr profile= App::getInstance()->getProfileConfig();
	VRDeviceViewPtr vrDeviceView =
		VRDeviceManager::getInstance()->getVRDeviceViewByPath(profile->originVRDevicePath);

	if (vrDeviceView != nullptr)
	{
		AnchorObjectSystemPtr anchorSystemPtr= AnchorObjectSystem::getSystem();
		AnchorComponentPtr originSpatialAnchor = anchorSystemPtr->getOriginSpatialAnchor();
		if (originSpatialAnchor)
		{
			const glm::mat4 devicePose = vrDeviceView->getCalibrationPose();

			glm::mat4 anchorXform = devicePose;
			if (profile->originVerticalAlignFlag)
			{
				const glm::vec3 deviceForward = glm_mat4_get_x_axis(devicePose);
				const glm::vec3 devicePosition = glm_mat4_get_position(devicePose);
				const glm::quat yawOnlyOrientation = glm::quatLookAt(deviceForward, glm::vec3(0.f, 1.f, 0.f));

				anchorXform = glm_mat4_from_pose(yawOnlyOrientation, devicePosition);
			}

			// Update origin anchor transform
			originSpatialAnchor->setWorldTransform(anchorXform);
		}
	}
}

void AnchorComponent::editAnchor()
{
	AnchorDefinitionPtr definition= getAnchorDefinition();
	MikanSpatialAnchorID anchorId= definition->getAnchorId();
	AnchorComponentPtr anchorComponent = AnchorObjectSystem::getSystem()->getSpatialAnchorById(anchorId);
	if (anchorComponent != nullptr)
	{
		// Show Anchor Triangulation Tool
		AppStage_AnchorTriangulation* anchorTriangulation = App::getInstance()->pushAppStage<AppStage_AnchorTriangulation>();
		
		AnchorTriangulatorInfo anchorInfo = {
			definition->getAnchorId(),
			definition->getRelativeTransform(),
			definition->getComponentName()
		};
		anchorTriangulation->setTargetAnchor(anchorInfo);
	}
}

void AnchorComponent::deleteAnchor()
{
	getOwnerObject()->deleteSelfConfig();
}