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
#include "TrackingVolumeObjectSystem.h"

// -- StageComponentDefinition -----
const std::string StageComponentDefinition::k_trackingVolumeIdPropertyId = "tracking_volume_id";

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
	pt[k_trackingVolumeIdPropertyId] = m_trackingVolumeId;

	return pt;
}

void StageComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_stageId = pt.get<int>("stage_id");
	m_trackingVolumeId = pt.get_or<int>(k_trackingVolumeIdPropertyId, INVALID_MIKAN_ID);
}

void StageComponentDefinition::setTrackingVolumeId(MikanTrackingVolumeID trackingVolumeId)
{
	if (m_trackingVolumeId != trackingVolumeId)
	{
		m_trackingVolumeId = trackingVolumeId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingVolumeIdPropertyId));
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

void StageComponent::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	TransformComponent::getPropertyNamesStatic(outPropertyNames);

	outPropertyNames.push_back(StageComponentDefinition::k_trackingVolumeIdPropertyId);
}

void StageComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	getPropertyNamesStatic(outPropertyNames);
}

bool StageComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (TransformComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_trackingVolumeIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::enumeration};
		return true;
	}

	return false;
}

bool StageComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (TransformComponent::getPropertyValue(propertyName, outValue))
		return true;

	StageComponentDefinitionConstPtr definition = getStageComponentDefinitionConst();
	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		outValue = (int)definition->getTrackingVolumeId();
		return true;
	}

	return false;
}

bool StageComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (TransformComponent::setPropertyValue(propertyName, inValue))
		return true;

	StageComponentDefinitionPtr definition = getStageComponentDefinition();
	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		definition->setTrackingVolumeId((MikanTrackingVolumeID)inValue.Get<int>());
		return true;
	}

	return false;
}

TrackingVolumeDefinitionConstPtr StageComponent::getTrackingVolumeDefinitionConst() const
{
	MikanTrackingVolumeID trackingVolumeId = getStageComponentDefinitionConst()->getTrackingVolumeId();
	if (trackingVolumeId != INVALID_MIKAN_ID)
	{
		ObjectSystemManager* systemManager = getOwnerObjectSystemManager();
		TrackingVolumeObjectSystemPtr trackingVolumeSystem = systemManager->getSystemOfType<TrackingVolumeObjectSystem>();
		if (trackingVolumeSystem)
		{
			return trackingVolumeSystem->getTrackingVolumeSystemConfigConst()->getTrackingVolumeDefinitionConst(trackingVolumeId);
		}
	}

	return nullptr;
}

// -- IFunctionInterface ----
const std::string StageComponent::k_alignStageFunctionId = "align_stage";
const std::string StageComponent::k_deleteStageFunctionId = "delete_stage";

void StageComponent::getFunctionNamesStatic(std::vector<std::string>& outPropertyNames)
{
	MikanComponent::getFunctionNamesStatic(outPropertyNames);

	outPropertyNames.push_back(k_alignStageFunctionId);
	outPropertyNames.push_back(k_deleteStageFunctionId);
}

void StageComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	getFunctionNamesStatic(outPropertyNames);
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
				MainWindow::getInstance()->pushAppStageOfType<AppStage_VRTrackingRecenter>();

			vrTrackingRecenterStage->setSourceCamera(cameraComponent);
			vrTrackingRecenterStage->setTargetStageId(stageId);
		});


}

void StageComponent::deleteStage()
{
	getOwnerObject()->deleteSelfConfig();
}