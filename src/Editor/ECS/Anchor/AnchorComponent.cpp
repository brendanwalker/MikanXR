#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "AnchorTriangulation/AppStage_AnchorTriangulation.h"
#include "CameraObjectSystem.h"
#include "EditorObjectSystem.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "App.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "IEditorWindow.h"
#include "MathGLM.h"
#include "ProjectConfig.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "SceneObjectSystem.h"
#include "MikanObject.h"
#include "MikanAnchorTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- AnchorConfig -----
AnchorDefinition::AnchorDefinition()
	: TransformComponentDefinition()
{
}

AnchorDefinition::AnchorDefinition(MikanSpatialAnchorID anchorId)
	: TransformComponentDefinition(anchorId)
{
}

configuru::Config AnchorDefinition::writeToJSON()
{
	return TransformComponentDefinition::writeToJSON();
}

void AnchorDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);
}

bool AnchorDefinition::readFromInitParams(
	MikanObjectSystem* ownerObjectSystem,
	const Serialization::PolymorphicObjectPtr& initParams)
{
	return TransformComponentDefinition::readFromInitParams(ownerObjectSystem, initParams);
}

// -- AnchorComponent -----
AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsCustomRender= true;
}

// -- IEntityAccessor ----
rfk::Struct const* AnchorComponent::getClientAPIValuesStructType() const
{
	return &MikanAnchorComponentValues::staticGetArchetype();
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
	auto editorObjectSystem= getObjectSystemOfType<EditorObjectSystem>();
	if (!editorObjectSystem->getEditorSettings().bDebugRenderAnchors)
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

	IMkGraphicsContext* graphicsContext = getGraphicsContext();
	drawTransformedAxes(graphicsContext, anchorXform, 0.1f, 0.1f, 0.1f, xColor, yColor, zColor);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszAnchorName);
}

// -- IFunctionInterface ----
const std::string AnchorComponent::k_editAnchorFunctionId = "edit_anchor";

void AnchorComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_editAnchorFunctionId, "Edit Anchor"));
}

bool AnchorComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == AnchorComponent::k_editAnchorFunctionId)
	{
		editAnchor();
		return true;
	}

	return TransformComponent::invokeFunction(functionName);
}

void AnchorComponent::editAnchor()
{
	AnchorDefinitionPtr definition= getAnchorDefinition();
	MikanSpatialAnchorID anchorId= definition->getComponentId();
	AnchorComponentPtr anchorComponent = 
		getObjectSystemOfType<AnchorObjectSystem>()->getSpatialAnchorById(anchorId);
	if (anchorComponent != nullptr)
	{
		AppStage* currentAppStage = getOwnerEditorWindow()->getCurrentAppStage();

		ModalDialog_SelectCamera::selectCamera(
			currentAppStage,
			[this, definition](MikanCameraID cameraId) {
				// Show Anchor Triangulation Tool
				AppStage_AnchorTriangulation* anchorTriangulation = 
					getOwnerEditorWindow()->pushAppStageOfType<AppStage_AnchorTriangulation>();

				AnchorTriangulatorInfo anchorInfo = {
					definition->getComponentId(),
					definition->getRelativeTransform(),
					definition->getComponentName()
				};
				anchorTriangulation->setSourceCamera(
					getObjectSystemOfType<CameraObjectSystem>()->getCameraById(cameraId));
				anchorTriangulation->setTargetAnchor(anchorInfo);
			});

	}
}