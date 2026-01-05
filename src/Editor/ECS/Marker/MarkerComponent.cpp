#include "MarkerComponent.h"
#include "App.h"
#include "CalibrationPatternFinder.h"
#include "Logger.h"
#include "MarkerObjectSystem.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanObject.h"
#include "OSUtils.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <hpdf.h>

#include <filesystem>

// -- MarkerDefinition -----
const std::string MarkerDefinition::k_arucoIdPropertyId= "aruco_id";
const std::string MarkerDefinition::k_lengthMMPropertyId= "length_mm";

MarkerDefinition::MarkerDefinition()
	: MikanComponentDefinition()
	, m_markerId(INVALID_MIKAN_ID)
	, m_arucoId(DEFAULT_ORIGIN_MARKER_ID)
	, m_lengthMM(DEFAULT_MARKER_SIZE_MM)
{
	
}

MarkerDefinition::MarkerDefinition(
	MikanMarkerID markerId,
	const std::string& markerName)
	: MikanComponentDefinition(markerId, markerName)
	, m_markerId(markerId)
	, m_arucoId(DEFAULT_ORIGIN_MARKER_ID)
	, m_lengthMM(DEFAULT_MARKER_SIZE_MM)
{}

configuru::Config MarkerDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_markerId;
	pt[MarkerDefinition::k_arucoIdPropertyId] = m_arucoId;
	pt[MarkerDefinition::k_lengthMMPropertyId] = m_lengthMM;

	return pt;
}

void MarkerDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_markerId = pt.get<int>("id");
		m_arucoId = pt.get_or<int>(MarkerDefinition::k_arucoIdPropertyId, m_arucoId);
		m_lengthMM = pt.get_or<float>(MarkerDefinition::k_lengthMMPropertyId, m_lengthMM);

		m_configName = StringUtils::stringify("Marker_", m_markerId);
	}
}

void MarkerDefinition::setArucoId(int arucoId)
{
	if (arucoId != m_arucoId)
	{
		m_arucoId = arucoId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_arucoIdPropertyId));
	}
}

void MarkerDefinition::setLengthMM(float lengthMM)
{
	if (lengthMM != m_lengthMM)
	{
		m_lengthMM = lengthMM;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_lengthMMPropertyId));
	}
}

// -- MarkerComponent -----
const std::string MarkerComponent::k_deleteMarkerFunctionId = "delete_marker";
const std::string MarkerComponent::k_printMarkerFunctionId = "print_marker";

MarkerComponent::MarkerComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
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
void MarkerComponent::getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			MarkerDefinition::k_arucoIdPropertyId, MikanVariantType::INT)
			->setDefaultValue(DEFAULT_ORIGIN_MARKER_ID));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			MarkerDefinition::k_lengthMMPropertyId, MikanVariantType::FLOAT)
			->setDefaultValue(DEFAULT_MARKER_SIZE_MM));
}

bool MarkerComponent::getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const
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

	return MikanComponent::getPropertyValue(propertyDesc, outValue);
}

bool MarkerComponent::setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == MarkerDefinition::k_arucoIdPropertyId)
	{
		getMarkerDefinition()->setArucoId(inValue.getIntValue());
		return true;
	}
	else if (propertyName == MarkerDefinition::k_lengthMMPropertyId)
	{
		getMarkerDefinition()->setLengthMM(inValue.getFloatValue());
		return true;
	}

	return MikanComponent::setPropertyValue(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
void MarkerComponent::getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_deleteMarkerFunctionId, "Delete Marker"));
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_printMarkerFunctionId, "Print Marker"));
}

bool MarkerComponent::invokeFunction(FunctionDescriptorConstPtr functionDesc)
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

	return MikanComponent::invokeFunction(functionDesc);
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
	MarkerObjectSystemPtr markerSystem = getOwnerMarkerSystem();
	ArucoDictionaryPtr dictionary =
		CalibrationPatternFinder::getArucoDictionary(
			markerSystem->getMarkerSystemConfig()->getArucoDictionaryType());

	MarkerDefinitionPtr markerDefinition = getMarkerDefinition();
	const int arucoId = markerDefinition->getArucoId();
	const float lengthMM = markerDefinition->getLengthMM();

	// Generate ArUco marker image using OpenCV
	cv::Mat markerImage;
	const int markerSizePixels = 600; // High resolution for printing
	cv::aruco::generateImageMarker(*dictionary.get(), arucoId, markerSizePixels, markerImage, 1);

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
