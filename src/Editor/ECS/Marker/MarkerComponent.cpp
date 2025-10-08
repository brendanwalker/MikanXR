#include "MarkerComponent.h"
#include "App.h"
#include "Logger.h"
#include "MarkerObjectSystem.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanObject.h"
#include "OSUtils.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <hpdf.h>

#include <filesystem>

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
		printMarker();
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
	MarkerDefinitionPtr markerDefinition = getMarkerDefinition();
	if (!markerDefinition)
	{
		MIKAN_LOG_ERROR("MarkerComponent::printMarker") << "No marker definition found";
		return;
	}

	const int arucoId = markerDefinition->getArucoId();
	const float lengthMM = markerDefinition->getLengthMM();

	// Get the ArUco dictionary type from the marker system
	MarkerObjectSystemPtr markerSystem = getOwnerMarkerSystem();
	if (!markerSystem)
	{
		MIKAN_LOG_ERROR("MarkerComponent::printMarker") << "No marker system found";
		return;
	}

	eCharucoDictionaryType charucoDictionaryType =
		markerSystem->getMarkerSystemConfig()->getArucoDictionaryType();

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

	// Generate ArUco marker image using OpenCV
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cvDictionaryType);
	cv::Mat markerImage;
	const int markerSizePixels = 600; // High resolution for printing
	cv::aruco::generateImageMarker(dictionary, arucoId, markerSizePixels, markerImage, 1);

	// Create PDF using libharu
	HPDF_Doc pdf = HPDF_New(NULL, NULL);
	if (!pdf)
	{
		MIKAN_LOG_ERROR("MarkerComponent::printMarker") << "Failed to create PDF document";
		return;
	}

	// Add a page (A4 size)
	HPDF_Page page = HPDF_AddPage(pdf);
	HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

	// Get page dimensions
	const float pageWidth = HPDF_Page_GetWidth(page);
	const float pageHeight = HPDF_Page_GetHeight(page);

	// Calculate marker size in points (1mm = 2.83465 points)
	const float mmToPoints = 2.83465f;
	const float markerSizePoints = lengthMM * mmToPoints;

	// Center the marker on the page
	const float xPos = (pageWidth - markerSizePoints) / 2.0f;
	const float yPos = (pageHeight - markerSizePoints) / 2.0f;

	// Convert OpenCV image to PNG format in memory
	std::vector<uchar> pngBuffer;
	cv::imencode(".png", markerImage, pngBuffer);

	// Load image from memory into libharu
	HPDF_Image image = HPDF_LoadPngImageFromMem(pdf, pngBuffer.data(), (HPDF_UINT)pngBuffer.size());
	if (!image)
	{
		MIKAN_LOG_ERROR("MarkerComponent::printMarker") << "Failed to load marker image into PDF";
		HPDF_Free(pdf);
		return;
	}

	// Draw the marker image on the page
	HPDF_Page_DrawImage(page, image, xPos, yPos, markerSizePoints, markerSizePoints);

	// Add text label below the marker
	HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
	HPDF_Page_SetFontAndSize(page, font, 12);

	std::string labelText = "ArUco Marker ID: " + std::to_string(arucoId) +
	                        " | Size: " + std::to_string((int)lengthMM) + "mm";
	const float textWidth = HPDF_Page_TextWidth(page, labelText.c_str());
	const float textXPos = (pageWidth - textWidth) / 2.0f;
	const float textYPos = yPos - 30.0f;

	HPDF_Page_BeginText(page);
	HPDF_Page_TextOut(page, textXPos, textYPos, labelText.c_str());
	HPDF_Page_EndText(page);

	// Save PDF to temp file
	std::filesystem::path pdfFilePath = std::filesystem::temp_directory_path() /
		("MikanMarker_" + std::to_string(arucoId) + ".pdf");

	HPDF_STATUS status = HPDF_SaveToFile(pdf, pdfFilePath.string().c_str());
	HPDF_Free(pdf);

	if (status != HPDF_OK)
	{
		MIKAN_LOG_ERROR("MarkerComponent::printMarker") << "Failed to save PDF to: " << pdfFilePath.string();
		return;
	}

	MIKAN_LOG_INFO("MarkerComponent::printMarker") << "PDF saved to: " << pdfFilePath.string();

	// Open PDF in default viewer (which will have print functionality)
	if (!OSUtils::openFileWithDefaultApplication(pdfFilePath))
	{
		MIKAN_LOG_ERROR("MarkerComponent::printMarker") << "Failed to open PDF viewer";
	}
}
