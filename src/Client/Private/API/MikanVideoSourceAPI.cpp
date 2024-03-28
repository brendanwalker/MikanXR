#include "MikanVideoSourceAPI.h"
#include "MikanRequestManager.h"
#include "MikanVideoSourceTypes_json.h"

MikanVideoSourceAPI::MikanVideoSourceAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
	m_requestManager->addResponseFactory<MikanVideoSourceIntrinsics>();
	m_requestManager->addResponseFactory<MikanVideoSourceMode>();
	m_requestManager->addResponseFactory<MikanVideoSourceAttachmentInfo>();
}

MikanResponseFuture MikanVideoSourceAPI::getVideoSourceIntrinsics()
{
	return m_requestManager->sendRequest(k_getVideoSourceIntrinsics);
}

MikanResponseFuture MikanVideoSourceAPI::getVideoSourceMode()
{
	return m_requestManager->sendRequest(k_getVideoSourceMode);
}

MikanResponseFuture MikanVideoSourceAPI::getVideoSourceAttachment()
{
	return m_requestManager->sendRequest(k_getVideoSourceAttachment);
}