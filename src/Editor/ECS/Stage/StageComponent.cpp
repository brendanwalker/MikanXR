#include "StageComponent.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanObject.h"
#include "MikanStageTypes.h"
#include "ProjectManager.h"
#include "SelectionComponent.h"
#include "TrackingVolumeQueries.h"
#include "TransformComponent.h"

#include <queue>

// -- StageComponentDefinition -----
const std::string StageComponentDefinition::k_trackingVolumeIdPropertyId = "tracking_volume_id";

StageComponentDefinition::StageComponentDefinition(
	MikanStageID sceneId)
	: TransformComponentDefinition(sceneId, "", glm_transform_to_MikanTransform(GlmTransform()))
{}

configuru::Config StageComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_trackingVolumeIdPropertyId] = m_trackingVolumeId;

	return pt;
}

void StageComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_trackingVolumeId = pt.get_or<int>(k_trackingVolumeIdPropertyId, INVALID_MIKAN_ID);
}

bool StageComponentDefinition::readFromInitParams(const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!TransformComponentDefinition::readFromInitParams(initParams))
		return false;

	const auto* componentValues = initParams.getTypedPointer<MikanStageComponentValues>();
	if (componentValues)
	{
		m_trackingVolumeId = componentValues->tracking_volume_id;
	}

	return true;
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

// -- IEntityAccessor ----
rfk::Struct const* StageComponent::getClientAPIValuesStructType() const
{
	return &MikanStageComponentValues::staticGetArchetype();
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

void StageComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			StageComponentDefinition::k_trackingVolumeIdPropertyId, MikanVariantType::INT)
			->setDefaultValue(-1));
}

bool StageComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		outValue = (int)getStageComponentDefinitionConst()->getTrackingVolumeId();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool StageComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == StageComponentDefinition::k_trackingVolumeIdPropertyId)
	{
		getStageComponentDefinition()->setTrackingVolumeId((MikanTrackingVolumeID)inValue.getIntValue());
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

TrackingVolumeComponentConstPtr StageComponent::getTrackingVolumeConst() const
{
	MikanTrackingVolumeID trackingVolumeId = getStageComponentDefinitionConst()->getTrackingVolumeId();
	if (trackingVolumeId != INVALID_MIKAN_ID)
	{
		ProjectManagerPtr projectManager = getOwnerProjectManager();
		return TrackingVolumeQueries::getTrackingVolumeById(projectManager, trackingVolumeId);
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

MikanStageID StageComponent::getStageId() const
{
	return getStageComponentDefinitionConst()->getComponentId();
}

void StageComponent::setTrackingVolumeId(MikanTrackingVolumeID volumeId)
{
	getStageComponentDefinition()->setTrackingVolumeId(volumeId);
}