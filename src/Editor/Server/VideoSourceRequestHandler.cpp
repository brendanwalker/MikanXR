#include "App.h"
#include "AppStage.h"
#include "VideoSourceRequestHandler.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVideoSourceRequests.h"
#include "ServerResponseHelpers.h"
#include "VideoSourceManager.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"

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

void VideoSourceRequestHandler::publishVideoSourceIntrinsicsChangedEvent()
{
	MikanCameraIntrinsicsChangedEvent intrinsicsChangeEvent = {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(intrinsicsChangeEvent));
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

	VideoSourceViewPtr videoSourceView =
		VideoSourceManager::getInstance()->getVideoSourceViewById(intrinsicsRequest.video_source_id);
	if (videoSourceView)
	{
		MikanVideoSourceIntrinsicsResponse intrinsicsResponse;
		videoSourceView->getCameraIntrinsics(intrinsicsResponse.intrinsics);

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

	VideoSourceViewPtr videoSourceView =
		VideoSourceManager::getInstance()->getVideoSourceViewById(modeRequest.video_source_id);
	if (videoSourceView)
	{
		const std::string devicePath = videoSourceView->getDevicePath();
		const IVideoSourceInterface::eDriverType driverType = videoSourceView->getVideoSourceDriverType();
		const VideoModeConfig* modeConfig = videoSourceView->getVideoMode();

		if (modeConfig != nullptr)
		{
			MikanVideoSourceModeResponse info;
			info.device_path = devicePath;
			info.frame_rate = modeConfig->frameRate;
			info.resolution_x = modeConfig->bufferPixelWidth;
			info.resolution_y = modeConfig->bufferPixelHeight;
			info.video_mode_name = modeConfig->modeName;
			switch (driverType)
			{
				case IVideoSourceInterface::OpenCV:
					info.video_source_api = MikanVideoSourceApi_INVALID;
					break;
				case IVideoSourceInterface::WindowsMediaFramework:
					info.video_source_api = MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION;
					break;
				case IVideoSourceInterface::INVALID:
				default:
					info.video_source_api = MikanVideoSourceApi_INVALID;
					break;
			}
			info.video_source_type = videoSourceView->getIsStereoCamera() ? MikanVideoSourceType_STEREO : MikanVideoSourceType_MONO;

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