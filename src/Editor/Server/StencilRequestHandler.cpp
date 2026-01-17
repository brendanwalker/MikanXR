#include "App.h"
#include "AppStage.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "StencilRequestHandler.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanStencilRequests.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "ProjectConfig.h"
#include "QuadStencilComponent.h"
#include "QuadStencilSystem.h"
#include "ServerResponseHelpers.h"
#include "StencilComponent.h"

#include <functional>
#include <easy/profiler.h>

using namespace std::placeholders;

// -- StencilRequestHandler -- //
bool StencilRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Stencil Requests
	messageServer->setRequestHandler(
		GetModelStencilRenderGeometry::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getModelStencilRenderGeometryHandler, this, _1, _2));

	return true;
}

void StencilRequestHandler::getModelStencilRenderGeometryHandler(const ClientRequest& request, ClientResponse& response)
{
	GetModelStencilRenderGeometry stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleBinaryResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	ModelStencilComponentPtr modelStencil =
		getObjectSystemOfType<ModelStencilSystem>()->getModelStencilById(stencilRequest.stencilId);
	if (modelStencil)
	{
		MikanStencilModelRenderGeometryResponse renderGeometryResponse = {};
		modelStencil->extractRenderGeometry(renderGeometryResponse.render_geometry);

		writeTypedBinaryResponse(request.requestId, renderGeometryResponse, response);
	}
	else
	{
		writeSimpleBinaryResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}