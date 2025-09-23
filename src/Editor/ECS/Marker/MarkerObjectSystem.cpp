#include "MarkerObjectSystem.h"
#include "App.h"
#include "Logger.h"
#include "MarkerComponent.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanObject.h"
#include "ObjectSystemManager.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include "RmlUi/Core/Variant.h"
#include "RmlUi/Config/Config.h"

// -- MarkerObjectSystemConfig -----
const std::string MarkerObjectSystemConfig::k_arucoMarkerListPropertyId= "arucoMarkers";
const std::string MarkerObjectSystemConfig::k_arucoIdListPropertyId = "arucoIdList";
const std::string MarkerObjectSystemConfig::k_arucoDictionaryTypePropertyId = "dictionaryType";
const std::string MarkerObjectSystemConfig::k_charucoRowsPropertyId = "charucoRows";
const std::string MarkerObjectSystemConfig::k_charucoColsPropertyId = "charucoCols";
const std::string MarkerObjectSystemConfig::k_charucoSquareLengthMMPropertyId = "charucoSquareLengthMM";
const std::string MarkerObjectSystemConfig::k_charucoMarkerLengthMMPropertyId = "charucoMarkerLengthMM";
const std::string MarkerObjectSystemConfig::k_charucoDictionaryTypePropertyId = "charucoDictionaryType";

configuru::Config MarkerObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextMarkerId"] = nextMarkerId;

	// ArUco settings
	pt["dictionaryType"] = k_charucoDictionaryStrings[(int)m_arucoDictionaryType];
	std::vector<configuru::Config> markerConfigs;
	for (MarkerDefinitionPtr MarkerDefinitionPtr : m_arucoMarkerList)
	{
		markerConfigs.push_back(MarkerDefinitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("arucoMarkers"), markerConfigs);

	// ChArUco settings
	pt["charucoRows"] = m_charucoRows;
	pt["charucoCols"] = m_charucoCols;
	pt["charucoSquareLengthMM"] = m_charucoSquareLengthMM;
	pt["charucoMarkerLengthMM"] = m_charucoMarkerLengthMM;
	pt["charucoDictionaryType"] = k_charucoDictionaryStrings[(int)m_charucoDictionaryType];

	return pt;
}

void MarkerObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	nextMarkerId = pt.get_or<int>("nextMarkerId", nextMarkerId);

	// Read in the ArUco settings
	const std::string charcuoDictionaryString =
		pt.get_or<std::string>(
			"dictionaryType",
			k_charucoDictionaryStrings[(int)eCharucoDictionaryType::DICT_6X6]);
	m_arucoDictionaryType =
		StringUtils::FindEnumValue<eCharucoDictionaryType>(
			charcuoDictionaryString,
			k_charucoDictionaryStrings);

	m_arucoMarkerList.clear();
	if (pt.has_key("arucoMarkers"))
	{
		for (const configuru::Config& marker_pt : pt["arucoMarkers"].as_array())
		{
			MarkerDefinitionPtr markerDefinitionPtr = std::make_shared<MarkerDefinition>();

			markerDefinitionPtr->readFromJSON(marker_pt);
			m_arucoMarkerList.push_back(markerDefinitionPtr);

			addChildConfig(markerDefinitionPtr);
		}
	}

	// Read ChAruco settings
	m_charucoRows = pt.get_or<int>("charucoRows", m_charucoRows);
	m_charucoCols = pt.get_or<int>("charucoCols", m_charucoCols);
	m_charucoSquareLengthMM = pt.get_or<float>("charucoSquareLengthMM", m_charucoSquareLengthMM);
	m_charucoMarkerLengthMM = pt.get_or<float>("charucoMarkerLengthMM", m_charucoMarkerLengthMM);

	const std::string charucoDictionaryString =
		pt.get_or<std::string>(
			"charucoDictionaryType",
			k_charucoDictionaryStrings[(int)eCharucoDictionaryType::DICT_6X6]);
	m_charucoDictionaryType =
		StringUtils::FindEnumValue<eCharucoDictionaryType>(
			charucoDictionaryString,
			k_charucoDictionaryStrings);
}

MarkerDefinitionPtr MarkerObjectSystemConfig::getMarkerConfig(MikanMarkerID markerId) const
{
	auto it = std::find_if(
		m_arucoMarkerList.begin(), m_arucoMarkerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
			return configPtr->getMarkerId() == markerId;
		});

	if (it != m_arucoMarkerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MarkerDefinitionPtr MarkerObjectSystemConfig::getMarkerConfigByName(const std::string& markerName) const
{
	auto it = std::find_if(
		m_arucoMarkerList.begin(), m_arucoMarkerList.end(),
		[markerName](MarkerDefinitionPtr configPtr) {
			return configPtr->getComponentName() == markerName;
		});

	if (it != m_arucoMarkerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MikanMarkerID MarkerObjectSystemConfig::addNewMarker()
{
	return addNewMarker("Marker_" + std::to_string(nextMarkerId));
}

MikanMarkerID MarkerObjectSystemConfig::addNewMarker(const std::string& markerName)
{
	MarkerDefinitionPtr MarkerDefinitionPtr = 
		std::make_shared<MarkerDefinition>(nextMarkerId, markerName);
	nextMarkerId++;

	m_arucoMarkerList.push_back(MarkerDefinitionPtr);
	addChildConfig(MarkerDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoMarkerListPropertyId));

	return MarkerDefinitionPtr->getMarkerId();
}

bool MarkerObjectSystemConfig::removeMarker(MikanMarkerID markerId)
{
	auto it = std::find_if(
		m_arucoMarkerList.begin(), m_arucoMarkerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
		return configPtr->getMarkerId() == markerId;
	});

	if (it != m_arucoMarkerList.end())
	{
		removeChildConfig(*it);

		m_arucoMarkerList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoMarkerListPropertyId));

		return true;
	}

	return false;
}

void MarkerObjectSystemConfig::setArucoDictionaryType(eCharucoDictionaryType dictionaryType)
{
	if (dictionaryType != m_arucoDictionaryType)
	{
		m_arucoDictionaryType = dictionaryType;
		markDirty(ConfigPropertyChangeSet()
			.addPropertyName(k_arucoDictionaryTypePropertyId)
			.addPropertyName(k_arucoIdListPropertyId));
	}
}

void MarkerObjectSystemConfig::getArucoIdList(std::vector<int>& outMarkerIdList) const
{
	outMarkerIdList.clear();

	int arucoMarkerCount = 0;
	switch (m_arucoDictionaryType)
	{
	case eCharucoDictionaryType::DICT_4X4:
		arucoMarkerCount = 4 * 4;
		break;
	case eCharucoDictionaryType::DICT_5X5:
		arucoMarkerCount = 5 * 5;
		break;
	case eCharucoDictionaryType::DICT_6X6:
		arucoMarkerCount = 6 * 6;
		break;
	case eCharucoDictionaryType::DICT_7X7:
		arucoMarkerCount = 7 * 7;
		break;
	}

	for (int i = 0; i < arucoMarkerCount; i++)
	{
		outMarkerIdList.push_back(i);
	}
}

void MarkerObjectSystemConfig::setCharucoRows(int charucoRows)
{
	if (charucoRows != m_charucoRows)
	{
		m_charucoRows = charucoRows;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoRowsPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoCols(int charucoCols)
{
	if (charucoCols != m_charucoCols)
	{
		m_charucoCols = charucoCols;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoColsPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoSquareLengthMM(float charucoSquareLengthMM)
{
	if (charucoSquareLengthMM != m_charucoSquareLengthMM)
	{
		m_charucoSquareLengthMM = charucoSquareLengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoSquareLengthMMPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoMarkerLengthMM(float charucoMarkerLengthMM)
{
	if (charucoMarkerLengthMM != m_charucoMarkerLengthMM)
	{
		m_charucoMarkerLengthMM = charucoMarkerLengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoMarkerLengthMMPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoDictionaryType(eCharucoDictionaryType charucoDictionaryType)
{
	if (charucoDictionaryType != m_charucoDictionaryType)
	{
		m_charucoDictionaryType = charucoDictionaryType;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoDictionaryTypePropertyId));
	}
}

// -- MarkerObjectSystem -----
bool MarkerObjectSystem::init()
{
	MikanObjectSystem::init();

	MarkerObjectSystemConfigConstPtr markerSystemConfig = getMarkerSystemConfigConst();

	for (MarkerDefinitionPtr markerConfig : markerSystemConfig->getArucoMarkerList())
	{
		createMarkerObject(markerConfig);
	}

	return true;
}

void MarkerObjectSystem::dispose()
{
	m_markerComponents.clear();

	MikanObjectSystem::dispose();
}

void MarkerObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	MarkerComponentPtr markerComponent = objectPtr->getComponentOfType<MarkerComponent>();
	if (markerComponent != nullptr)
	{
		removeMarker(markerComponent->getMarkerDefinition()->getMarkerId());
	}
}

MarkerComponentPtr MarkerObjectSystem::getMarkerById(MikanMarkerID markerId) const
{
	auto iter = m_markerComponents.find(markerId);
	if (iter != m_markerComponents.end())
	{
		return iter->second.lock();
	}

	return MarkerComponentPtr();
}

MarkerComponentPtr MarkerObjectSystem::getMarkerByName(const std::string& markerName) const
{
	for (auto it = m_markerComponents.begin(); it != m_markerComponents.end(); it++)
	{
		MarkerComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getName() == markerName)
		{
			return componentPtr;
		}
	}

	return MarkerComponentPtr();
}

MarkerComponentPtr MarkerObjectSystem::addNewMarker()
{
	MarkerObjectSystemConfigPtr markerSystemConfig = getMarkerSystemConfig();

	MikanMarkerID markerId = markerSystemConfig->addNewMarker();
	if (markerId != INVALID_MIKAN_ID)
	{
		MarkerDefinitionPtr markerConfig = markerSystemConfig->getMarkerConfig(markerId);
		assert(markerConfig != nullptr);

		return createMarkerObject(markerConfig);
	}

	return MarkerComponentPtr();
}

bool MarkerObjectSystem::removeMarker(MikanMarkerID markerId)
{
	getMarkerSystemConfig()->removeMarker(markerId);
	disposeMarkerObject(markerId);

	return true;
}

MarkerComponentPtr MarkerObjectSystem::createMarkerObject(MarkerDefinitionPtr markerConfig)
{
	MikanObjectPtr markerObject = newObject();
	markerObject->setName(markerConfig->getComponentName());

	// Add marker component to the object
	MarkerComponentPtr markerComponentPtr = markerObject->addComponent<MarkerComponent>();
	markerComponentPtr->setDefinition(markerConfig);
	m_markerComponents.insert({markerConfig->getMarkerId(), markerComponentPtr});

	// Init the object once all components are added
	markerObject->init();

	return markerComponentPtr;
}

void MarkerObjectSystem::disposeMarkerObject(MikanMarkerID markerId)
{
	auto it = m_markerComponents.find(markerId);
	if (it != m_markerComponents.end())
	{
		MarkerComponentPtr markerComponentPtr = it->second.lock();

		// Remove from component list
		m_markerComponents.erase(it);

		// Free the corresponding object
		deleteObject(markerComponentPtr->getOwnerObject());
	}
}

MarkerObjectSystemConfigConstPtr MarkerObjectSystem::getMarkerSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->markerSystemConfig;
}

MarkerObjectSystemConfigPtr MarkerObjectSystem::getMarkerSystemConfig()
{
	return std::const_pointer_cast<MarkerObjectSystemConfig>(getMarkerSystemConfigConst());
}

// -- IRmlPropertyInterface ----
void MarkerObjectSystem::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerObjectSystemConfig::k_arucoDictionaryTypePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerObjectSystemConfig::k_charucoDictionaryTypePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerObjectSystemConfig::k_charucoRowsPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerObjectSystemConfig::k_charucoColsPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerObjectSystemConfig::k_charucoSquareLengthMMPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			MarkerObjectSystemConfig::k_charucoMarkerLengthMMPropertyId));
}

bool MarkerObjectSystem::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MarkerObjectSystemConfig::k_arucoDictionaryTypePropertyId)
	{
		outValue = k_charucoDictionaryStrings[(int)getMarkerSystemConfigConst()->getArucoDictionaryType()];
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoDictionaryTypePropertyId)
	{
		outValue = k_charucoDictionaryStrings[(int)getMarkerSystemConfigConst()->getCharucoDictionaryType()];
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoRowsPropertyId)
	{
		outValue = getMarkerSystemConfigConst()->getCharucoRows();
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoColsPropertyId)
	{
		outValue = getMarkerSystemConfigConst()->getCharucoCols();
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoSquareLengthMMPropertyId)
	{
		outValue = getMarkerSystemConfigConst()->getCharucoSquareLengthMM();
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoMarkerLengthMMPropertyId)
	{
		outValue = getMarkerSystemConfigConst()->getCharucoMarkerLengthMM();
		return true;
	}

	return MikanObjectSystem::getPropertyValueFromRml(propertyDesc, outValue);
}

bool MarkerObjectSystem::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MarkerObjectSystemConfig::MarkerObjectSystemConfig::k_arucoDictionaryTypePropertyId)
	{
		const std::string dictionaryString = inValue.Get<std::string>();
		eCharucoDictionaryType dictionaryType =
			StringUtils::FindEnumValue<eCharucoDictionaryType>(
				dictionaryString,
				k_charucoDictionaryStrings);
		getMarkerSystemConfig()->setArucoDictionaryType(dictionaryType);
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoDictionaryTypePropertyId)
	{
		const std::string dictionaryString = inValue.Get<std::string>();
		eCharucoDictionaryType dictionaryType =
			StringUtils::FindEnumValue<eCharucoDictionaryType>(
				dictionaryString,
				k_charucoDictionaryStrings);
		getMarkerSystemConfig()->setCharucoDictionaryType(dictionaryType);
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoRowsPropertyId)
	{
		int charucoRows = inValue.Get<int>();
		getMarkerSystemConfig()->setCharucoRows(charucoRows);
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoColsPropertyId)
	{
		int charucoCols = inValue.Get<int>();
		getMarkerSystemConfig()->setCharucoCols(charucoCols);
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoSquareLengthMMPropertyId)
	{
		float charucoSquareLengthMM = inValue.Get<float>();
		getMarkerSystemConfig()->setCharucoSquareLengthMM(charucoSquareLengthMM);
		return true;
	}
	else if (propertyName == MarkerObjectSystemConfig::k_charucoMarkerLengthMMPropertyId)
	{
		float charucoMarkerLengthMM = inValue.Get<float>();
		getMarkerSystemConfig()->setCharucoMarkerLengthMM(charucoMarkerLengthMM);
		return true;
	}

	return MikanObjectSystem::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
const std::string MarkerObjectSystem::k_printCharucoMarkerFunctionId = "print_marker";

void MarkerObjectSystem::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanObjectSystem::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_printCharucoMarkerFunctionId, "Print Marker"));
}

bool MarkerObjectSystem::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == k_printCharucoMarkerFunctionId)
	{
		//TODO
		return true;
	}

	return MikanObjectSystem::invokeFunctionFromRml(functionDesc);
}