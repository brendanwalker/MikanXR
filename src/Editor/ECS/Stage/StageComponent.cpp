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
	: TransformComponentDefinition(sceneId, componentName, glm_transform_to_MikanTransform(GlmTransform()))
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

void StageComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			StageComponentDefinition::k_trackingVolumeIdPropertyId));
}

bool StageComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		outValue = (int)getStageComponentDefinitionConst()->getTrackingVolumeId();
		return true;
	}

	return TransformComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool StageComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		getStageComponentDefinition()->setTrackingVolumeId((MikanTrackingVolumeID)inValue.Get<int>());
		return true;
	}

	return TransformComponent::setPropertyValueFromRml(propertyDesc, inValue);
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

// -- IRmlFunctionInterface ----
const std::string StageComponent::k_alignStageFunctionId = "align_stage";
const std::string StageComponent::k_deleteStageFunctionId = "delete_stage";

void StageComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_alignStageFunctionId, "Align Stage"));
	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteStageFunctionId, "Delete Stage"));
}

bool StageComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionId = functionDesc->getFunctionName();

	if (functionId == k_alignStageFunctionId)
	{
		alignStage();
		return true;
	}
	else if (functionId == k_deleteStageFunctionId)
	{
		deleteStage();
		return true;
	}

	return MikanComponent::invokeFunctionFromRml(functionDesc);
}

void StageComponent::setTrackingVolumeId(MikanTrackingVolumeID volumeId)
{
	getStageComponentDefinition()->setTrackingVolumeId(volumeId);
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