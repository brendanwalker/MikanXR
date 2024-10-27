#include "MikanVideoSourceAPI.h"
#include "MikanRequestManager.h"

MikanVideoSourceAPI::MikanVideoSourceAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
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