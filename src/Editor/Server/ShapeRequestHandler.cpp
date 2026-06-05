#include "App.h"
#include "AppStage.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanShapeRequests.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "ProjectConfig.h"
#include "ServerResponseHelpers.h"
#include "ShapeRequestHandler.h"

#include <functional>
#include <easy/profiler.h>

using namespace std::placeholders;

// -- ShapeRequestHandler -- //
bool ShapeRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Shape Requests
	messageServer->setRequestHandler(
		GetModelShapeRenderGeometry::staticGetArchetype().getName(),
		std::bind(&ShapeRequestHandler::getModelShapeRenderGeometryHandler, this, _1, _2));

	return true;
}

void ShapeRequestHandler::getModelShapeRenderGeometryHandler(const ClientRequest& request, ClientResponse& response)
{
	GetModelShapeRenderGeometry shapeRequest;
	if (!readTypedRequest(request.utf8RequestString, shapeRequest))
	{
		writeSimpleBinaryResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	ModelShapeComponentPtr modelShape =
		getObjectSystemOfType<ModelShapeSystem>()->getModelShapeById(shapeRequest.shapeId);
	if (modelShape)
	{
		MikanShapeModelRenderGeometryResponse renderGeometryResponse = {};
		modelShape->extractRenderGeometry(renderGeometryResponse.render_geometry);

		writeTypedBinaryResponse(request.requestId, renderGeometryResponse, response);
	}
	else
	{
		writeSimpleBinaryResponse(request.requestId, MikanAPIResult::InvalidShapeID, response);
	}
}
