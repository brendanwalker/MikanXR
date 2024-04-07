#include "MikanSpatialAnchorAPI.h"
#include "MikanSpatialAnchorTypes_json.h"
#include "MikanRequestManager.h"

MikanSpatialAnchorAPI::MikanSpatialAnchorAPI(class MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
	m_requestManager->addResponseFactory<MikanSpatialAnchorList>();
	m_requestManager->addResponseFactory<MikanSpatialAnchorInfo>();
}

MikanResponseFuture MikanSpatialAnchorAPI::getSpatialAnchorList()
{
	return m_requestManager->sendRequest(k_getSpatialAnchorList);
}

MikanResponseFuture MikanSpatialAnchorAPI::getSpatialAnchorInfo(MikanSpatialAnchorID anchorId)
{
	return m_requestManager->sendRequestWithPayload<int>(k_getSpatialAnchorInfo, anchorId);
}

MikanResponseFuture MikanSpatialAnchorAPI::findSpatialAnchorInfoByName(const std::string& anchorName)
{
	return m_requestManager->sendRequestWithPayload<std::string>(k_findSpatialAnchorInfoByName, anchorName);
}

