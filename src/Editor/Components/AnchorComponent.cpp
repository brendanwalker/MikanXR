#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "App.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "ProfileConfig.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MikanSpatialAnchorTypes.h"
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

// -- IFunctionInterface ----
const std::string AnchorComponent::k_editAnchorFunctionId = "edit_anchor";
const std::string AnchorComponent::k_deleteAnchorFunctionId = "delete_anchor";

void AnchorComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	SceneComponent::getFunctionNames(outPropertyNames);

	AnchorObjectSystemPtr anchorSystemPtr = AnchorObjectSystem::getSystem();

	outPropertyNames.push_back(k_editAnchorFunctionId);
	outPropertyNames.push_back(k_deleteAnchorFunctionId);
}

bool AnchorComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (SceneComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == AnchorComponent::k_editAnchorFunctionId)
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

	if (functionName == AnchorComponent::k_editAnchorFunctionId)
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
		// Show Anchor Triangulation Tool
		AppStage_AnchorTriangulation* anchorTriangulation = MainWindow::getInstance()->pushAppStage<AppStage_AnchorTriangulation>();
		
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