#include "CalibrationPatternFinder.h"
#include "Logger.h"
#include "MarkerObjectSystem.h"
#include "MarkerRequestHandler.h"
#include "MikanMarkerRequests.h"
#include "MikanServer.h"
#include "ServerResponseHelpers.h"
#include "StringUtils.h"

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect/aruco_detector.hpp"

#include <functional>

using namespace std::placeholders;

// -- MarkerRequestHandler -- //
bool MarkerRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	messageServer->setRequestHandler(
		GetArucoMarkerImageRequest::staticGetArchetype().getName(),
		std::bind(&MarkerRequestHandler::getArucoMarkerImageHandler, this, _1, _2));

	return true;
}

void MarkerRequestHandler::getArucoMarkerImageHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetArucoMarkerImageRequest imageRequest;
	if (!readTypedRequest(request.utf8RequestString, imageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto markerSystem = getObjectSystemOfType<MarkerObjectSystem>();
	if (!markerSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const eCharucoDictionaryType dictionaryType =
		markerSystem->getTypedDefinitionConst()->getArucoDictionaryType();

	ArucoDictionaryPtr dictionary = CalibrationPatternFinder::getArucoDictionary(dictionaryType);
	if (!dictionary)
	{
		MIKAN_LOG_ERROR("MarkerRequestHandler::getArucoMarkerImageHandler")
			<< "Failed to get ArUco dictionary for type: " << (int)dictionaryType;
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::GeneralError, response);
		return;
	}

	const int markerId = imageRequest.markerId;
	const int imageSize = imageRequest.imageSize > 0 ? imageRequest.imageSize : 200;

	// Clamp marker ID to valid range
	const int maxMarkerId = dictionary->bytesList.rows - 1;
	const int clampedMarkerId = std::max(0, std::min(markerId, maxMarkerId));

	try
	{
		cv::Mat markerImage;
		cv::aruco::generateImageMarker(*dictionary, clampedMarkerId, imageSize, markerImage, 1);

		std::vector<uint8_t> pngBuffer;
		if (!cv::imencode(".png", markerImage, pngBuffer))
		{
			MIKAN_LOG_ERROR("MarkerRequestHandler::getArucoMarkerImageHandler")
				<< "Failed to encode marker image to PNG";
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::GeneralError, response);
			return;
		}

		ArucoMarkerImageResponse imageResponse;
		imageResponse.imageData.setValue(StringUtils::base64Encode(pngBuffer));
		writeTypedJsonResponse(request.requestId, imageResponse, response);
	}
	catch (const cv::Exception& e)
	{
		MIKAN_LOG_ERROR("MarkerRequestHandler::getArucoMarkerImageHandler")
			<< "OpenCV exception generating marker: " << e.what();
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::GeneralError, response);
	}
}
