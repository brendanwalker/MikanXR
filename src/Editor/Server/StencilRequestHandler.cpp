#include "App.h"
#include "AppStage.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "StencilRequestHandler.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanStencilEvents.h"
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

	// Stencil Events
	getProjectConfig()->quadStencilSystemDefinition->OnPropertyChanged +=
		MakeDelegate(this, &StencilRequestHandler::handleStencilSystemConfigChange);
	getProjectConfig()->boxStencilSystemDefinition->OnPropertyChanged +=
		MakeDelegate(this, &StencilRequestHandler::handleStencilSystemConfigChange);
	getProjectConfig()->modelStencilSystemDefinition->OnPropertyChanged +=
		MakeDelegate(this, &StencilRequestHandler::handleStencilSystemConfigChange);

	// Stencil Requests
	messageServer->setRequestHandler(
		GetQuadStencilList::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getQuadStencilListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetQuadStencil::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getQuadStencilHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetBoxStencilList::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getBoxStencilListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetBoxStencil::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getBoxStencilHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetModelStencilList::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getModelStencilListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetModelStencil::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getModelStencilHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetModelStencilRenderGeometry::staticGetArchetype().getId(),
		std::bind(&StencilRequestHandler::getModelStencilRenderGeometryHandler, this, _1, _2));

	return true;
}

void StencilRequestHandler::shutdown()
{
	getProjectConfig()->quadStencilSystemDefinition->OnPropertyChanged -=
		MakeDelegate(this, &StencilRequestHandler::handleStencilSystemConfigChange);
	getProjectConfig()->boxStencilSystemDefinition->OnPropertyChanged -=
		MakeDelegate(this, &StencilRequestHandler::handleStencilSystemConfigChange);
	getProjectConfig()->modelStencilSystemDefinition->OnPropertyChanged -=
		MakeDelegate(this, &StencilRequestHandler::handleStencilSystemConfigChange);
}

// Stencil Events
void StencilRequestHandler::handleStencilSystemConfigChange(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		auto anchorConfig = std::static_pointer_cast<StencilComponentDefinition>(configPtr);

		MikanStencilNameUpdateEvent nameUpdateEvent;
		nameUpdateEvent.stencil_id = anchorConfig->getStencilId();
		nameUpdateEvent.stencil_name = anchorConfig->getComponentName();

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(nameUpdateEvent));
	}
	else if (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativePositionPropertyId) ||
			 changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeRotationPropertyId) ||
			 changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeScalePropertyId))
	{
		auto anchorConfig = std::static_pointer_cast<StencilComponentDefinition>(configPtr);

		MikanStencilPoseUpdateEvent poseUpdateEvent;
		poseUpdateEvent.stencil_id = anchorConfig->getStencilId();
		poseUpdateEvent.transform = glm_transform_to_MikanTransform(anchorConfig->getRelativeTransform());

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(poseUpdateEvent));
	}
	else if (changedPropertySet.hasPropertyName(QuadStencilSystemDefinition::k_quadStencilListPropertyId))
	{
		MikanQuadStencilListUpdateEvent listUpdateEvent= {};

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(listUpdateEvent));
	}
	else if (changedPropertySet.hasPropertyName(BoxStencilSystemDefinition::k_boxStencilListPropertyId))
	{
		MikanBoxStencilListUpdateEvent listUpdateEvent = {};

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(listUpdateEvent));
	}
	else if (changedPropertySet.hasPropertyName(ModelStencilSystemDefinition::k_modelStencilListPropertyId))
	{
		MikanModelStencilListUpdateEvent listUpdateEvent = {};

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(listUpdateEvent));
	}
}

void StencilRequestHandler::getQuadStencilListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilListResponse stencilListResult = {};

	auto quadStencilSystemDefinition = getProjectConfig()->quadStencilSystemDefinition;
	for (QuadStencilDefinitionPtr quadConfig : quadStencilSystemDefinition->getQuadStencilList())
	{
		stencilListResult.stencil_id_list.push_back(quadConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void StencilRequestHandler::getQuadStencilHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetQuadStencil stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto quadStencilSystemDefinition = getProjectConfig()->quadStencilSystemDefinition;
	auto quadConfig = quadStencilSystemDefinition->getQuadStencilConfigConst(stencilRequest.stencilId);
	if (quadConfig != nullptr)
	{
		MikanStencilQuadInfoResponse stencilResponse = {};
		stencilResponse.quad_info = quadConfig->getQuadInfo();

		writeTypedJsonResponse(request.requestId, stencilResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}

void StencilRequestHandler::getBoxStencilListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilListResponse stencilListResult = {};

	auto boxStencilSystemDefinition = getProjectConfig()->boxStencilSystemDefinition;
	for (BoxStencilDefinitionPtr boxConfig : boxStencilSystemDefinition->getBoxStencilList())
	{
		stencilListResult.stencil_id_list.push_back(boxConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void StencilRequestHandler::getBoxStencilHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetBoxStencil stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto boxStencilSystemDefinition = getProjectConfig()->boxStencilSystemDefinition;
	auto boxConfig = boxStencilSystemDefinition->getBoxStencilConfigConst(stencilRequest.stencilId);
	if (boxConfig != nullptr)
	{
		MikanStencilBoxInfoResponse stencilResponse;
		stencilResponse.box_info = boxConfig->getBoxInfo();

		writeTypedJsonResponse(request.requestId, stencilResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}

void StencilRequestHandler::getModelStencilListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilListResponse stencilListResult = {};

	auto modelStencilSystemDefinition = getProjectConfig()->modelStencilSystemDefinition;
	for (ModelStencilDefinitionPtr modelConfig : modelStencilSystemDefinition->getModelStencilList())
	{
		stencilListResult.stencil_id_list.push_back(modelConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void StencilRequestHandler::getModelStencilHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetModelStencil stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto modelStencilSystemDefinition = getProjectConfig()->modelStencilSystemDefinition;
	auto modelConfig = modelStencilSystemDefinition->getModelStencilConfigConst(stencilRequest.stencilId);
	if (modelConfig != nullptr)
	{
		MikanStencilModelInfoResponse stencilResponse = {};
		stencilResponse.model_info = modelConfig->getModelInfo();

		writeTypedJsonResponse(request.requestId, stencilResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
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