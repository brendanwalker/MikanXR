#include "CameraObjectSystem.h"
#include "StageComponent.h"
#include "ModalSelectCamera/ModalDialog_SelectCamera.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"
#include "VRTrackingRecenter/AppStage_VRTrackingRecenter.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <queue>
#include "TrackingSystemsConfig.h"

// -- StageComponentDefinition -----
const std::string StageComponentDefinition::k_trackingSystemIdPropertyId = "tracking_system_id";

StageComponentDefinition::StageComponentDefinition(
	MikanStageID sceneId,
	const std::string& componentName)
	: TransformComponentDefinition(componentName, glm_transform_to_MikanTransform(GlmTransform()))
	, m_stageId(sceneId)
{}

configuru::Config StageComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt["stage_id"] = m_stageId;
	pt[k_trackingSystemIdPropertyId] = m_trackingSystemId;

	return pt;
}

void StageComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_stageId = pt.get<int>("stage_id");
	m_trackingSystemId = pt.get_or<int>(k_trackingSystemIdPropertyId, INVALID_MIKAN_ID);
}

void StageComponentDefinition::setTrackingSystemId(MikanTrackingSystemID systemId)
{
	if (m_trackingSystemId != systemId)
	{
		m_trackingSystemId = systemId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingSystemIdPropertyId));
	}
}

// -- StageComponent -----
StageComponent::StageComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
}

void StageComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	TransformComponent::setDefinition(definition);
}

void StageComponent::init()
{
	TransformComponent::init();
}

void StageComponent::dispose()
{
	TransformComponent::dispose();
}

void StageComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(StageComponentDefinition::k_trackingSystemIdPropertyId);
}

bool StageComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (TransformComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == StageComponentDefinition::k_trackingSystemIdPropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_trackingSystemIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::enumeration};
		return true;
	}

	return false;
}

bool StageComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (TransformComponent::getPropertyValue(propertyName, outValue))
		return true;

	StageComponentDefinitionConstPtr definition = getStageComponentDefinitionConst();
	if (propertyName == StageComponentDefinition::k_trackingSystemIdPropertyId)
	{
		outValue = (int)definition->getTrackingSystemId();
		return true;
	}

	return false;
}

bool StageComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (TransformComponent::setPropertyValue(propertyName, inValue))
		return true;

	StageComponentDefinitionPtr definition = getStageComponentDefinition();
	if (propertyName == StageComponentDefinition::k_trackingSystemIdPropertyId)
	{
		definition->setTrackingSystemId((MikanTrackingSystemID)inValue.Get<int>());
		return true;
	}

	return false;
}

TrackingSystemDefinitionConstPtr StageComponent::getTrackingSystemDefinitionConst() const
{
	MikanTrackingSystemID systemId = getStageComponentDefinitionConst()->getTrackingSystemId();
	if (systemId != INVALID_MIKAN_ID)
	{
		return TrackingSystemsConfig::getSystemConfig()->getTrackingSystemDefinitionConst(systemId);
	}

	return nullptr;
}

// -- IFunctionInterface ----
const std::string StageComponent::k_alignStageFunctionId = "align_stage";
const std::string StageComponent::k_deleteStageFunctionId = "delete_stage";

void StageComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_alignStageFunctionId);
	outPropertyNames.push_back(k_deleteStageFunctionId);
}

bool StageComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (TransformComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == StageComponent::k_alignStageFunctionId)
	{
		outDescriptor = {StageComponent::k_alignStageFunctionId, "Align Stage"};
		return true;
	}
	else if (functionName == StageComponent::k_deleteStageFunctionId)
	{
		outDescriptor = {StageComponent::k_deleteStageFunctionId, "Delete Stage"};
		return true;
	}

	return false;
}

bool StageComponent::invokeFunction(const std::string& functionName)
{
	if (TransformComponent::invokeFunction(functionName))
		return true;

	if (functionName == StageComponent::k_alignStageFunctionId)
	{
		alignStage();
	}
	else if (functionName == StageComponent::k_deleteStageFunctionId)
	{
		deleteStage();
	}

	return false;
}

void StageComponent::alignStage()
{
	ModalDialog_SelectCamera::selectCamera(
		[this](MikanCameraID cameraId) {
			const MikanStageID stageId = getStageComponentDefinition()->getStageId();
			CameraComponentPtr cameraComponent= CameraObjectSystem::getSystem()->getCameraById(cameraId);

			AppStage_VRTrackingRecenter* vrTrackingRecenterStage =
				MainWindow::getInstance()->pushAppStage<AppStage_VRTrackingRecenter>();

			vrTrackingRecenterStage->setSourceCamera(cameraComponent);
			vrTrackingRecenterStage->setTargetStageId(stageId);
		});


}

void StageComponent::deleteStage()
{
	getOwnerObject()->deleteSelfConfig();
}