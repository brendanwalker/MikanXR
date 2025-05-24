#include "StageComponent.h"
#include "MikanObject.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanCamera.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <queue>

// -- StageComponentDefinition -----
const std::string StageComponentDefinition::k_trackingSystemPropertyId = "tracking_system";
const std::string StageComponentDefinition::k_originMarkerIdPropertyId = "origin_marker_id";
const std::string StageComponentDefinition::k_originMarkerSizePropertyId = "origin_marker_size";
const std::string StageComponentDefinition::k_utilityMarkerIdPropertyId = "utility_marker_id";
const std::string StageComponentDefinition::k_utilityMarkerSizePropertyId = "utility_marker_size";

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
	pt[k_trackingSystemPropertyId] = (int)m_trackingSystem;
	pt[k_originMarkerIdPropertyId] = m_originMarkerId;
	pt[k_originMarkerSizePropertyId] = m_originMarkerSizeMM;
	pt[k_utilityMarkerIdPropertyId] = m_utilityMarkerId;
	pt[k_utilityMarkerSizePropertyId] = m_utilityMarkerSizeMM;

	return pt;
}

void StageComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_stageId = pt.get<int>("stage_id");
	m_trackingSystem = (MikanStageTrackingSystem)pt.get<int>("k_trackingSystemPropertyId");
	m_originMarkerId = pt.get<int>(k_originMarkerIdPropertyId);
	m_originMarkerSizeMM = pt.get<float>(k_originMarkerSizePropertyId);
	m_utilityMarkerId = pt.get<int>(k_utilityMarkerIdPropertyId);
	m_utilityMarkerSizeMM = pt.get<float>(k_utilityMarkerSizePropertyId);
}

void StageComponentDefinition::setTrackingSystem(MikanStageTrackingSystem system)
{
	if (m_trackingSystem != system)
	{
		m_trackingSystem = system;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingSystemPropertyId));
	}
}

void StageComponentDefinition::setOriginMarkerId(MikanMarkerID markerId)
{
	if (m_originMarkerId != markerId)
	{
		m_originMarkerId = markerId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_originMarkerIdPropertyId));
	}
}

void StageComponentDefinition::setOriginMarkerSize(float size)
{
	if (m_originMarkerSizeMM != size)
	{
		m_originMarkerSizeMM = size;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_originMarkerSizePropertyId));
	}
}

void StageComponentDefinition::setUtilityMarkerId(MikanMarkerID markerId)
{
	if (m_utilityMarkerId != markerId)
	{
		m_utilityMarkerId = markerId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_utilityMarkerIdPropertyId));
	}
}

void StageComponentDefinition::setUtilityMarkerSize(float size)
{
	if (m_utilityMarkerSizeMM != size)
	{
		m_utilityMarkerSizeMM = size;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_utilityMarkerSizePropertyId));
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

	outPropertyNames.push_back(StageComponentDefinition::k_trackingSystemPropertyId);
	outPropertyNames.push_back(StageComponentDefinition::k_originMarkerIdPropertyId);
	outPropertyNames.push_back(StageComponentDefinition::k_originMarkerSizePropertyId);
	outPropertyNames.push_back(StageComponentDefinition::k_utilityMarkerIdPropertyId);
	outPropertyNames.push_back(StageComponentDefinition::k_utilityMarkerSizePropertyId);
}

bool StageComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (TransformComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == StageComponentDefinition::k_trackingSystemPropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_trackingSystemPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::enumeration};
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_originMarkerIdPropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_originMarkerIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::marker_id};
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_utilityMarkerIdPropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_utilityMarkerIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::marker_id};
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_originMarkerSizePropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_originMarkerSizePropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d};
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_utilityMarkerSizePropertyId)
	{
		outDescriptor = {StageComponentDefinition::k_utilityMarkerSizePropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d};
		return true;
	}

	return false;
}

bool StageComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (TransformComponent::getPropertyValue(propertyName, outValue))
		return true;

	StageComponentDefinitionPtr definition = getStageComponentDefinition();
	if (propertyName == StageComponentDefinition::k_trackingSystemPropertyId)
	{
		outValue = (int)definition->getTrackingSystem();
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_originMarkerIdPropertyId)
	{
		outValue = definition->getOriginMarkerId();
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_utilityMarkerIdPropertyId)
	{
		outValue = definition->getUtilityMarkerId();
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_originMarkerSizePropertyId)
	{
		outValue = definition->getOriginMarkerSize();
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_utilityMarkerSizePropertyId)
	{
		outValue = definition->getUtilityMarkerSize();
		return true;
	}

	return false;
}

bool StageComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (TransformComponent::setPropertyValue(propertyName, inValue))
		return true;

	StageComponentDefinitionPtr definition = getStageComponentDefinition();
	if (propertyName == StageComponentDefinition::k_trackingSystemPropertyId)
	{
		definition->setTrackingSystem((MikanStageTrackingSystem)inValue.Get<int>());
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_originMarkerIdPropertyId)
	{
		definition->setOriginMarkerId(inValue.Get<int>());
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_utilityMarkerIdPropertyId)
	{
		definition->setUtilityMarkerId(inValue.Get<int>());
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_originMarkerSizePropertyId)
	{
		definition->setOriginMarkerSize(inValue.Get<float>());
		return true;
	}
	else if (propertyName == StageComponentDefinition::k_utilityMarkerSizePropertyId)
	{
		definition->setUtilityMarkerSize(inValue.Get<float>());
		return true;
	}

	return false;
}

// -- IFunctionInterface ----
const std::string StageComponent::k_deleteStageFunctionId = "delete_stage";

void StageComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	TransformComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_deleteStageFunctionId);
}

bool StageComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (TransformComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == StageComponent::k_deleteStageFunctionId)
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

	if (functionName == StageComponent::k_deleteStageFunctionId)
	{
		deleteStage();
	}

	return false;
}

void StageComponent::deleteStage()
{
	getOwnerObject()->deleteSelfConfig();
}