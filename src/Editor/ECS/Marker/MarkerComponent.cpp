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
	: MikanComponentDefinition(markerName)
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

void MarkerComponent::getFunctionNamesStatic(std::vector<std::string>& outPropertyNames)
{
	MikanComponent::getFunctionNamesStatic(outPropertyNames);
	outPropertyNames.push_back(k_deleteMarkerFunctionId);
}

void MarkerComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	getFunctionNamesStatic(outPropertyNames);
}

bool MarkerComponent::getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const
{
	if (functionName == k_deleteMarkerFunctionId)
	{
		outDescriptor = FunctionDescriptor(k_deleteMarkerFunctionId, "Delete Marker");
		return true;
	}

	return MikanComponent::getFunctionDescriptor(functionName, outDescriptor);
}

bool MarkerComponent::invokeFunction(const std::string& functionName)
{
	bool bHandled = false;

	if (functionName == k_deleteMarkerFunctionId)
	{
		deleteMarker();
		bHandled = true;
	}

	if (!bHandled)
	{
		bHandled = MikanComponent::invokeFunction(functionName);
	}

	return bHandled;
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

void MarkerComponent::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	MikanComponent::getPropertyNamesStatic(outPropertyNames);

	outPropertyNames.push_back(MarkerDefinition::k_arucoIdPropertyId);
	outPropertyNames.push_back(MarkerDefinition::k_lengthMMPropertyId);
}

void MarkerComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	getPropertyNamesStatic(outPropertyNames);
}

bool MarkerComponent::getPropertyDescriptor(
	const std::string& propertyName,
	PropertyDescriptor& outDescriptor) const
{
	if (propertyName == MarkerDefinition::k_arucoIdPropertyId)
	{
		outDescriptor = { MarkerDefinition::k_arucoIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::marker_id };
		return true;
	}
	else if (propertyName == MarkerDefinition::k_lengthMMPropertyId)
	{
		outDescriptor = { MarkerDefinition::k_lengthMMPropertyId, ePropertyDataType::datatype_float, ePropertySemantic::size1d };
		return true;
	}

	return MikanComponent::getPropertyDescriptor(propertyName, outDescriptor);
}

bool MarkerComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
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

	return MikanComponent::getPropertyValue(propertyName, outValue);
}

bool MarkerComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
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

	return MikanComponent::setPropertyValue(propertyName, inValue);
}
