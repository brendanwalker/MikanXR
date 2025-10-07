#include "MarkerComponent.h"
#include "App.h"
#include "Logger.h"
#include "MarkerObjectSystem.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanObject.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- MarkerDefinition -----
const std::string MarkerDefinition::k_arucoIdPropertyId= "aruco_id";
const std::string MarkerDefinition::k_lengthMMPropertyId= "length_mm";

MarkerDefinition::MarkerDefinition()
	: MikanComponentDefinition()
{
	m_markerId = INVALID_MIKAN_ID;
}

MarkerDefinition::MarkerDefinition(
	MikanMarkerID markerId,
	const std::string& markerName)
	: MikanComponentDefinition(markerId, markerName)
	, m_markerId(markerId)
{}

configuru::Config MarkerDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_markerId;
	pt["aruco_id"] = m_arucoId;
	pt["length_mm"] = m_lengthMM;

	return pt;
}

void MarkerDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_markerId = pt.get<int>("id");
		m_arucoId = pt.get_or<int>("aruco_id", 0);
		m_lengthMM = pt.get_or<float>("length_mm", 100.0f); // Default length is 100mm

		m_configName = StringUtils::stringify("Marker_", m_markerId);
	}
}

void MarkerDefinition::setArucoId(int arucoId)
{
	if (arucoId != m_arucoId)
	{
		m_arucoId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoIdPropertyId));
	}
}

void MarkerDefinition::setLengthMM(float lengthMM)
{
	if (lengthMM != m_lengthMM)
	{
		m_lengthMM = lengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_lengthMMPropertyId));
	}
}

// -- MarkerComponent -----
const std::string MarkerComponent::k_deleteMarkerFunctionId = "delete_marker";
const std::string MarkerComponent::k_printMarkerFunctionId = "print_marker";

MarkerComponent::MarkerComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

void MarkerComponent::init()
{
	MikanComponent::init();

	// Get the selection component that should be on the same object
	MikanObjectPtr ownerObject = getOwnerObject();
	if (ownerObject)
	{
		m_selectionComponent = ownerObject->getComponentOfType<SelectionComponent>();
	}
}

MarkerObjectSystemPtr MarkerComponent::getOwnerMarkerSystem() const
{
	return std::static_pointer_cast<MarkerObjectSystem>(getOwnerObject()->getOwnerSystem());
}

//TODO
//void MarkerComponent::extractMarkerInfoForClientAPI(struct MikanMarkerInfo& outMarkerInfo) const
//{
//	MarkerDefinitionPtr markerDefinition = getMarkerDefinition();
//	if (markerDefinition)
//	{
//		outMarkerInfo.marker_id = markerDefinition->getMarkerId();
//		outMarkerInfo.aruco_id = markerDefinition->getArucoId();
//		outMarkerInfo.length_mm = markerDefinition->getLengthMM();
//	}
//}

// -- IRmlPropertyInterface ----
void MarkerComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerDefinition::k_arucoIdPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerDefinition::k_lengthMMPropertyId));
}

bool MarkerComponent::getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MarkerDefinition::k_arucoIdPropertyId)
	{
		outValue = getMarkerDefinition()->getArucoId();
		return true;
	}
	else if (propertyName == MarkerDefinition::k_lengthMMPropertyId)
	{
		outValue = getMarkerDefinition()->getLengthMM();
		return true;
	}

	return MikanComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool MarkerComponent::setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MarkerDefinition::k_arucoIdPropertyId)
	{
		getMarkerDefinition()->setArucoId(inValue.Get<int>());
		return true;
	}
	else if (propertyName == MarkerDefinition::k_lengthMMPropertyId)
	{
		getMarkerDefinition()->setLengthMM(inValue.Get<float>());
		return true;
	}

	return MikanComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
void MarkerComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteMarkerFunctionId, "Delete Marker"));
	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_printMarkerFunctionId, "Print Marker"));
}

bool MarkerComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	if (functionDesc->getFunctionName() == k_deleteMarkerFunctionId)
	{
		deleteMarker();
		return true;
	}
	else if (functionDesc->getFunctionName() == k_printMarkerFunctionId)
	{
		deleteMarker();
		return true;
	}

	return MikanComponent::invokeFunctionFromRml(functionDesc);
}

void MarkerComponent::deleteMarker()
{
	MarkerDefinitionPtr markerDefinition = getMarkerDefinition();
	if (markerDefinition)
	{
		const MikanMarkerID markerId = markerDefinition->getMarkerId();

		getOwnerMarkerSystem()->removeMarker(markerId);
	}
}

void MarkerComponent::printMarker()
{
	//TODO
}
