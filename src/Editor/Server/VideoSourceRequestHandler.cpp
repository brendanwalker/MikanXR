#include "App.h"
#include "AppStage.h"
#include "VideoSourceRequestHandler.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVideoSourceRequests.h"
#include "ProjectManager.h"
#include "ServerResponseHelpers.h"
#include "VideoSourceQueries.h"
#include "VideoSourceComponent.h"

#include <functional>

using namespace std::placeholders;

// -- VideoSourceRequestHandler -- //
bool VideoSourceRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Video Source Requests
	messageServer->setRequestHandler(
		GetVideoSourceIntrinsics::staticGetArchetype().getId(),
		std::bind(&VideoSourceRequestHandler::getVideoSourceIntrinsicsHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetVideoSourceMode::staticGetArchetype().getId(),
		std::bind(&VideoSourceRequestHandler::getVideoSourceModeHandler, this, _1, _2));

	return true;
}

// Video Source Events
void VideoSourceRequestHandler::publishVideoSourceOpenedEvent()
{
	MikanVideoSourceOpenedEvent openedEvent = {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(openedEvent));
}

void VideoSourceRequestHandler::publishVideoSourceClosedEvent()
{
	MikanVideoSourceClosedEvent closedEvent = {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(closedEvent));
}

void VideoSourceRequestHandler::publishVideoSourceModeChangedEvent()
{
	MikanVideoSourceModeChangedEvent modeChangeEvent = {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(modeChangeEvent));
}

// Video Source Requests
void VideoSourceRequestHandler::getVideoSourceIntrinsicsHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetVideoSourceIntrinsics intrinsicsRequest;
	if (!readTypedRequest(request.utf8RequestString, intrinsicsRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	VideoSourceComponentPtr videoSourceComponent =
		VideoSourceQueries::getVideoSourceById(
			getProjectManager(),
			intrinsicsRequest.video_source_id);
	if (videoSourceComponent)
	{
		MikanVideoSourceIntrinsicsResponse intrinsicsResponse;
		videoSourceComponent->getCameraIntrinsics(intrinsicsResponse.intrinsics);

		writeTypedJsonResponse(request.requestId, intrinsicsResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
	}
}

void VideoSourceRequestHandler::getVideoSourceModeHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetVideoSourceMode modeRequest;
	if (!readTypedRequest(request.utf8RequestString, modeRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	VideoSourceComponentPtr videoSourceComponent =
		VideoSourceQueries::getVideoSourceById(
			getProjectManager(),
			modeRequest.video_source_id);
	if (videoSourceComponent)
	{
		const std::string devicePath = videoSourceComponent->getDevicePath();
		const std::string deviceAPI = videoSourceComponent->getDeviceAPI();

		std::string videoModeName;
		int pixelWidth, pixelHeight;
		float frameRate = 0.0f;
		if (videoSourceComponent->getVideoModeName(videoModeName) &&
			videoSourceComponent->getVideoPixelDimensions(pixelWidth, pixelHeight) &&
			videoSourceComponent->getFrameRate(frameRate))
		{
			MikanVideoSourceModeResponse info;

			info.device_path = devicePath;
			info.frame_rate = frameRate;
			info.resolution_x = pixelWidth;
			info.resolution_y = pixelHeight;
			info.video_mode_name = videoModeName;
			info.video_source_api = deviceAPI;
			// TODO: For the moment we only support monoscopic cameras.
			info.video_source_type = MikanVideoSourceType_MONO;

			writeTypedJsonResponse(request.requestId, info, response);
		}
		else
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
		}
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
	}
}