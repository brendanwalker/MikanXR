#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "CameraObjectSystem.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
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
#include "MikanSpatialAnchorTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- AnchorConfig -----
AnchorDefinition::AnchorDefinition()
	: TransformComponentDefinition()
{
	m_anchorId = INVALID_MIKAN_ID;
}

AnchorDefinition::AnchorDefinition(
	MikanSpatialAnchorID anchorId,
	const std::string& anchorName,
	const MikanTransform& xform)
	: TransformComponentDefinition(anchorId, StringUtils::stringify("Anchor_", anchorId), xform)
	, m_anchorId(anchorId)
{
}

configuru::Config AnchorDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["id"] = m_anchorId;

	return pt;
}

void AnchorDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_anchorId = pt.get<int>("id");
		m_configName = StringUtils::stringify("Anchor_", m_anchorId);
	}
}

// -- AnchorComponent -----
AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
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
	wchar_t wszAnchorName[256];
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

// -- IRmlPropertyInterface ----
void AnchorComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getRmlPropertyDescriptors(outDescriptors);
}

// -- IRmlFunctionInterface ----
const std::string AnchorComponent::k_editAnchorFunctionId = "edit_anchor";
const std::string AnchorComponent::k_deleteAnchorFunctionId = "delete_anchor";

void AnchorComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_editAnchorFunctionId, "Edit Anchor"));
	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteAnchorFunctionId, "Delete Anchor"));
}

bool AnchorComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == AnchorComponent::k_editAnchorFunctionId)
	{
		editAnchor();
		return true;
	}
	else if (functionName == AnchorComponent::k_deleteAnchorFunctionId)
	{
		deleteAnchor();
		return true;
	}

	return TransformComponent::invokeFunctionFromRml(functionDesc);
}

void AnchorComponent::extractAnchorInfoForClientAPI(MikanSpatialAnchorInfo& outAnchorInfo) const
{
	const std::string anchorName = getName();
	const GlmTransform anchorWorldTransform(getWorldTransform());
	const MikanSpatialAnchorID anchorId = getAnchorDefinition()->getAnchorId();

	outAnchorInfo.anchor_id = getAnchorDefinition()->getAnchorId();
	outAnchorInfo.anchor_name= anchorName;
	outAnchorInfo.world_transform = glm_transform_to_MikanTransform(anchorWorldTransform);
}

void AnchorComponent::editAnchor()
{
	AnchorDefinitionPtr definition= getAnchorDefinition();
	MikanSpatialAnchorID anchorId= definition->getAnchorId();
	AnchorComponentPtr anchorComponent = AnchorObjectSystem::getSystem()->getSpatialAnchorById(anchorId);
	if (anchorComponent != nullptr)
	{
		ModalDialog_SelectCamera::selectCamera(
			[this, definition](MikanCameraID cameraId) {
				// Show Anchor Triangulation Tool
				AppStage_AnchorTriangulation* anchorTriangulation = 
					MainWindow::getInstance()->pushAppStageOfType<AppStage_AnchorTriangulation>();

				AnchorTriangulatorInfo anchorInfo = {
					definition->getAnchorId(),
					definition->getRelativeTransform(),
					definition->getComponentName()
				};
				anchorTriangulation->setSourceCamera(
					CameraObjectSystem::getSystem()->getCameraById(cameraId));
				anchorTriangulation->setTargetAnchor(anchorInfo);
			});

	}
}

void AnchorComponent::deleteAnchor()
{
	getOwnerObject()->deleteSelfConfig();
}