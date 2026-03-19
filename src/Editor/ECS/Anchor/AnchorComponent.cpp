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
#include "SceneObjectSystem.h"
#include "MikanObject.h"
#include "MikanAnchorTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- AnchorConfig -----
const std::string AnchorDefinition::k_ownerSceneIdPropertyId = "owner_scene_id";
AnchorDefinition::AnchorDefinition()
	: TransformComponentDefinition()
{
	m_ownerSceneId = INVALID_MIKAN_ID;
}

AnchorDefinition::AnchorDefinition(
	MikanSpatialAnchorID anchorId)
	: TransformComponentDefinition(anchorId)
{
}

configuru::Config AnchorDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_ownerSceneIdPropertyId] = m_ownerSceneId;
	return pt;
}

void AnchorDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_ownerSceneId = pt.get_or<int>(k_ownerSceneIdPropertyId, m_ownerSceneId);
}

bool AnchorDefinition::readFromInitParams(const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TransformComponentDefinition::readFromInitParams(initParams))
		return false;

	const auto* componentValues = initParams.getTypedPointer<MikanAnchorComponentValues>();
	if (componentValues)
	{
		m_ownerSceneId = componentValues->owner_scene_id;
	}

	return true;
}

void AnchorDefinition::setOwnerSceneId(MikanSceneID sceneId)
{
	if (sceneId != m_ownerSceneId)
	{
		m_ownerSceneId = sceneId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_ownerSceneIdPropertyId));
	}
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
	auto anchorObjectSystem= getObjectSystemOfType<AnchorObjectSystem>();
	const auto anchorSystemConfig= anchorObjectSystem->getTypedDefinitionConst();
	if (!anchorSystemConfig->getRenderAnchorsFlag())
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

// -- IPropertyInterface ----
void AnchorComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			AnchorDefinition::k_ownerSceneIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(INVALID_MIKAN_ID)
		->setReadOnly());
}

bool AnchorComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == AnchorDefinition::k_ownerSceneIdPropertyId)
	{
		outValue = static_cast<int>(getAnchorDefinition()->getOwnerSceneId());
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
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

SceneComponentConstPtr AnchorComponent::getOwnerSceneComponent() const
{
	MikanSceneID sceneId = getAnchorDefinition()->getOwnerSceneId();

	return getObjectSystemOfType<SceneObjectSystem>()->getSceneById(sceneId);
}

void AnchorComponent::editAnchor()
{
	AnchorDefinitionPtr definition= getAnchorDefinition();
	MikanSpatialAnchorID anchorId= definition->getComponentId();
	AnchorComponentPtr anchorComponent = 
		getObjectSystemOfType<AnchorObjectSystem>()->getSpatialAnchorById(anchorId);
	if (anchorComponent != nullptr)
	{
		ModalDialog_SelectCamera::selectCamera(
			[this, definition](MikanCameraID cameraId) {
				// Show Anchor Triangulation Tool
				AppStage_AnchorTriangulation* anchorTriangulation = 
					MainWindow::getInstance()->pushAppStageOfType<AppStage_AnchorTriangulation>();

				AnchorTriangulatorInfo anchorInfo = {
					definition->getComponentId(),
					definition->getOwnerSceneId(),
					definition->getRelativeTransform(),
					definition->getComponentName()
				};
				anchorTriangulation->setSourceCamera(
					getObjectSystemOfType<CameraObjectSystem>()->getCameraById(cameraId));
				anchorTriangulation->setTargetAnchor(anchorInfo);
			});

	}
}