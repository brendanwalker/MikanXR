#include "App.h"
#include "AppStage.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "AnchorRequestHandler.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanSpatialAnchorEvents.h"
#include "MikanSpatialAnchorRequests.h"
#include "ProjectConfig.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

// -- AnchorRequestHandler -- //
bool AnchorRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	AnchorObjectSystem::getSystem()->getAnchorSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &AnchorRequestHandler::handleAnchorSystemConfigChange);

	// Spatial Anchor Requests
	messageServer->setRequestHandler(
		GetSpatialAnchorList::staticGetArchetype().getId(),
		std::bind(&AnchorRequestHandler::getSpatialAnchorListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetSpatialAnchorInfo::staticGetArchetype().getId(),
		std::bind(&AnchorRequestHandler::getSpatialAnchorInfoHandler, this, _1, _2));
	messageServer->setRequestHandler(
		FindSpatialAnchorInfoByName::staticGetArchetype().getId(),
		std::bind(&AnchorRequestHandler::findSpatialAnchorInfoByNameHandler, this, _1, _2));

	return true;
}

void AnchorRequestHandler::shutdown()
{
	AnchorObjectSystem::getSystem()->getAnchorSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &AnchorRequestHandler::handleAnchorSystemConfigChange);
}

void AnchorRequestHandler::handleAnchorSystemConfigChange(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		AnchorDefinitionPtr anchorConfig = std::static_pointer_cast<AnchorDefinition>(configPtr);

		MikanAnchorNameUpdateEvent nameUpdateEvent;
		nameUpdateEvent.anchor_id = anchorConfig->getAnchorId();
		nameUpdateEvent.anchor_name = anchorConfig->getComponentName();

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(nameUpdateEvent));
	}
	else if (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativePositionPropertyId) ||
			 changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeRotationPropertyId) ||
			 changedPropertySet.hasPropertyName(TransformComponentDefinition::k_relativeScalePropertyId))
	{
		AnchorDefinitionPtr anchorConfig = std::static_pointer_cast<AnchorDefinition>(configPtr);

		MikanAnchorPoseUpdateEvent poseUpdateEvent;
		poseUpdateEvent.anchor_id = anchorConfig->getAnchorId();
		poseUpdateEvent.transform = glm_transform_to_MikanTransform(anchorConfig->getRelativeTransform());

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(poseUpdateEvent));
	}
	else if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		MikanAnchorListUpdateEvent listUpdateEvent= {};

		m_owner->publishMikanJsonEvent(mikanTypeToJsonString(listUpdateEvent));
	}
}


void AnchorRequestHandler::getSpatialAnchorListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanSpatialAnchorListResponse anchorListResult = {};

	auto anchorSystemConfig = getProjectConfig()->anchorConfig;
	for (AnchorDefinitionPtr spatialAnchor : anchorSystemConfig->spatialAnchorList)
	{
		anchorListResult.spatial_anchor_id_list.push_back(spatialAnchor->getAnchorId());
	}

	writeTypedJsonResponse(request.requestId, anchorListResult, response);
}

void AnchorRequestHandler::getSpatialAnchorInfoHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetSpatialAnchorInfo anchorRequest;
	if (!readTypedRequest(request.utf8RequestString, anchorRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorById(anchorRequest.anchorId);
	if (anchorPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidAnchorID, response);
		return;
	}

	MikanSpatialAnchorInfoResponse anchorInfoResponse = {};
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfoResponse.anchor_info);

	writeTypedJsonResponse(request.requestId, anchorInfoResponse, response);
}

void AnchorRequestHandler::findSpatialAnchorInfoByNameHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	FindSpatialAnchorInfoByName anchorRequest;
	if (!readTypedRequest(request.utf8RequestString, anchorRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& anchorName = anchorRequest.anchorName.getValue();
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorByName(anchorName);
	if (anchorPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidAnchorID, response);
		return;
	}

	MikanSpatialAnchorInfoResponse anchorInfoResponse = {};
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfoResponse.anchor_info);

	writeTypedJsonResponse(request.requestId, anchorInfoResponse, response);
}