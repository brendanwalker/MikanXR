#include "StageComponent.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"

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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_trackingVolumeIdPropertyId));
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

void StageComponent::getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			StageComponentDefinition::k_trackingVolumeIdPropertyId, MikanVariantType::INT)
			->setDefaultValue(-1));
}

bool StageComponent::getPropertyValue(
	PropertyDescriptorConstPtr propertyDesc,
	MikanVariant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		outValue = (int)getStageComponentDefinitionConst()->getTrackingVolumeId();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyDesc, outValue);
}

bool StageComponent::setPropertyValue(
	PropertyDescriptorConstPtr propertyDesc,
	const MikanVariant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		getStageComponentDefinition()->setTrackingVolumeId((MikanTrackingVolumeID)inValue.getIntValue());
		return true;
	}

	return TransformComponent::setPropertyValue(propertyDesc, inValue);
}

TrackingVolumeComponentConstPtr StageComponent::getTrackingVolumeConst() const
{
	MikanTrackingVolumeID trackingVolumeId = getStageComponentDefinitionConst()->getTrackingVolumeId();
	if (trackingVolumeId != INVALID_MIKAN_ID)
	{
		ProjectManagerPtr systemManager = getOwnerProjectManager();
		TrackingVolumeObjectSystemPtr trackingVolumeSystem = systemManager->getSystemOfType<TrackingVolumeObjectSystem>();
		if (trackingVolumeSystem)
		{
			return trackingVolumeSystem->getTrackingVolumeById(trackingVolumeId);
		}
	}

	return TrackingVolumeComponentConstPtr();
}

TrackingVolumeDefinitionConstPtr StageComponent::getTrackingVolumeDefinitionConst() const
{
	TrackingVolumeComponentConstPtr trackingVolume= getTrackingVolumeConst();
	if (trackingVolume)
	{
		return trackingVolume->getTrackingVolumeDefinition();
	}

	return TrackingVolumeDefinitionConstPtr();
}

// -- IRmlFunctionInterface ----
const std::string StageComponent::k_deleteStageFunctionId = "delete_stage";

void StageComponent::getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_deleteStageFunctionId, "Delete Stage"));
}

bool StageComponent::invokeFunction(FunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionId = functionDesc->getFunctionName();

	if (functionId == k_deleteStageFunctionId)
	{
		deleteStage();
		return true;
	}

	return MikanComponent::invokeFunction(functionDesc);
}

MikanStageID StageComponent::getStageId() const
{
	return getStageComponentDefinitionConst()->getStageId();
}

void StageComponent::setTrackingVolumeId(MikanTrackingVolumeID volumeId)
{
	getStageComponentDefinition()->setTrackingVolumeId(volumeId);
}

void StageComponent::deleteStage()
{
	getOwnerObject()->deleteSelfConfig();
}