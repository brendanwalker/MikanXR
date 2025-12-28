#include "MarkerObjectSystem.h"
#include "App.h"
#include "Logger.h"
#include "MarkerComponent.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanObject.h"
#include "ProjectManager.h"
#include "OSUtils.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include "RmlUi/Core/Variant.h"
#include "RmlUi/Config/Config.h"

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <hpdf.h>

#include <filesystem>

// -- MarkerObjectSystemConfig -----
const std::string MarkerObjectSystemConfig::k_arucoMarkerListPropertyId= "arucoMarkers";
const std::string MarkerObjectSystemConfig::k_arucoIdListPropertyId = "arucoIdList";
const std::string MarkerObjectSystemConfig::k_arucoDictionaryTypePropertyId = "arucoDictionaryType";
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
	pt["dictionaryType"] = k_charucoDictionaryStrings[(int)m_arucoDictionaryType].c_str();
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
	pt["charucoDictionaryType"] = k_charucoDictionaryStrings[(int)m_charucoDictionaryType].c_str();

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

	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_arucoMarkerListPropertyId));

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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_arucoMarkerListPropertyId));

		return true;
	}

	return false;
}

void MarkerObjectSystemConfig::setArucoDictionaryType(eCharucoDictionaryType dictionaryType)
{
	if (dictionaryType != m_arucoDictionaryType)
	{
		m_arucoDictionaryType = dictionaryType;
		notifyPropertyChanged(ConfigPropertyChangeSet()
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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_charucoRowsPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoCols(int charucoCols)
{
	if (charucoCols != m_charucoCols)
	{
		m_charucoCols = charucoCols;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_charucoColsPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoSquareLengthMM(float charucoSquareLengthMM)
{
	if (charucoSquareLengthMM != m_charucoSquareLengthMM)
	{
		m_charucoSquareLengthMM = charucoSquareLengthMM;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_charucoSquareLengthMMPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoMarkerLengthMM(float charucoMarkerLengthMM)
{
	if (charucoMarkerLengthMM != m_charucoMarkerLengthMM)
	{
		m_charucoMarkerLengthMM = charucoMarkerLengthMM;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_charucoMarkerLengthMMPropertyId));
	}
}

void MarkerObjectSystemConfig::setCharucoDictionaryType(eCharucoDictionaryType charucoDictionaryType)
{
	if (charucoDictionaryType != m_charucoDictionaryType)
	{
		m_charucoDictionaryType = charucoDictionaryType;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_charucoDictionaryTypePropertyId));
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
	return getProjectConfig()->markerSystemConfig;
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
const std::string MarkerObjectSystem::k_printCharucoMarkerFunctionId = "print_checkerboard";

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
		printMarker();
		return true;
	}

	return MikanObjectSystem::invokeFunctionFromRml(functionDesc);
}

void MarkerObjectSystem::printMarker()
{
	MarkerObjectSystemConfigPtr markerSystemConfig = getMarkerSystemConfig();
	if (!markerSystemConfig)
	{
		MIKAN_LOG_ERROR("MarkerObjectSystem::printMarker") << "No marker system config found";
		return;
	}

	// Get ChArUco board parameters from config
	const int charucoRows = markerSystemConfig->getCharucoRows();
	const int charucoCols = markerSystemConfig->getCharucoCols();
	const float squareLengthMM = markerSystemConfig->getCharucoSquareLengthMM();
	const float markerLengthMM = markerSystemConfig->getCharucoMarkerLengthMM();
	const eCharucoDictionaryType charucoDictionaryType = markerSystemConfig->getCharucoDictionaryType();

	// Convert to OpenCV dictionary type
	cv::aruco::PredefinedDictionaryType cvDictionaryType = cv::aruco::DICT_6X6_250;
	switch (charucoDictionaryType)
	{
		case eCharucoDictionaryType::DICT_4X4:
			cvDictionaryType = cv::aruco::DICT_4X4_250;
			break;
		case eCharucoDictionaryType::DICT_5X5:
			cvDictionaryType = cv::aruco::DICT_5X5_250;
			break;
		case eCharucoDictionaryType::DICT_6X6:
			cvDictionaryType = cv::aruco::DICT_6X6_250;
			break;
		case eCharucoDictionaryType::DICT_7X7:
			cvDictionaryType = cv::aruco::DICT_7X7_250;
			break;
		default:
			break;
	}

	// Create ChArUco board
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cvDictionaryType);
	cv::aruco::CharucoBoard charucoBoard(
		cv::Size(charucoCols, charucoRows),
		squareLengthMM,
		markerLengthMM,
		dictionary);

	// Generate ChArUco board image
	cv::Mat boardImage;
	const int boardSizePixels = 2400; // High resolution for printing
	charucoBoard.generateImage(cv::Size(boardSizePixels, boardSizePixels), boardImage, 10, 1);

	// Create PDF using libharu
	HPDF_Doc pdf = HPDF_New(NULL, NULL);
	if (!pdf)
	{
		MIKAN_LOG_ERROR("MarkerObjectSystem::printMarker") << "Failed to create PDF document";
		return;
	}

	// Add a page (A4 size, landscape to fit board better)
	HPDF_Page page = HPDF_AddPage(pdf);
	HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE);

	// Get page dimensions
	const float pageWidth = HPDF_Page_GetWidth(page);
	const float pageHeight = HPDF_Page_GetHeight(page);

	// Calculate board size in points (1mm = 2.83465 points)
	const float mmToPoints = 2.83465f;
	const float boardWidthMM = charucoCols * squareLengthMM;
	const float boardHeightMM = charucoRows * squareLengthMM;
	const float boardWidthPoints = boardWidthMM * mmToPoints;
	const float boardHeightPoints = boardHeightMM * mmToPoints;

	// Center the board on the page
	const float xPos = (pageWidth - boardWidthPoints) / 2.0f;
	const float yPos = (pageHeight - boardHeightPoints) / 2.0f;

	// Convert OpenCV image to PNG format in memory
	std::vector<uchar> pngBuffer;
	cv::imencode(".png", boardImage, pngBuffer);

	// Load image from memory into libharu
	HPDF_Image image = HPDF_LoadPngImageFromMem(pdf, pngBuffer.data(), (HPDF_UINT)pngBuffer.size());
	if (!image)
	{
		MIKAN_LOG_ERROR("MarkerObjectSystem::printMarker") << "Failed to load board image into PDF";
		HPDF_Free(pdf);
		return;
	}

	// Draw the board image on the page
	HPDF_Page_DrawImage(page, image, xPos, yPos, boardWidthPoints, boardHeightPoints);

	// Add text label below the board
	HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
	HPDF_Page_SetFontAndSize(page, font, 12);

	std::string labelText = "ChArUco Board: " + std::to_string(charucoRows) + "x" + std::to_string(charucoCols) +
	                        " | Square: " + std::to_string((int)squareLengthMM) + "mm" +
	                        " | Marker: " + std::to_string((int)markerLengthMM) + "mm";
	const float textWidth = HPDF_Page_TextWidth(page, labelText.c_str());
	const float textXPos = (pageWidth - textWidth) / 2.0f;
	const float textYPos = yPos - 30.0f;

	HPDF_Page_BeginText(page);
	HPDF_Page_TextOut(page, textXPos, textYPos, labelText.c_str());
	HPDF_Page_EndText(page);

	// Save PDF to temp file
	std::filesystem::path pdfFilePath = std::filesystem::temp_directory_path() /
		("MikanCharucoBoard_" + std::to_string(charucoRows) + "x" + std::to_string(charucoCols) + ".pdf");

	HPDF_STATUS status = HPDF_SaveToFile(pdf, pdfFilePath.string().c_str());
	HPDF_Free(pdf);

	if (status != HPDF_OK)
	{
		MIKAN_LOG_ERROR("MarkerObjectSystem::printMarker") << "Failed to save PDF to: " << pdfFilePath.string();
		return;
	}

	MIKAN_LOG_INFO("MarkerObjectSystem::printMarker") << "PDF saved to: " << pdfFilePath.string();

	// Open PDF in default viewer (which will have print functionality)
	if (!OSUtils::openFileWithDefaultApplication(pdfFilePath))
	{
		MIKAN_LOG_ERROR("MarkerObjectSystem::printMarker") << "Failed to open PDF viewer";
	}
}